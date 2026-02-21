#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#define DIOIN     2
#define CLKIN     3
#define DIOOUT    4
#define CLKOUT    5

// 31B is just a random value, should never be hit.
#define MAX_DATA_BYTES 31

typedef struct {
    uint8_t data[MAX_DATA_BYTES];
    uint8_t length;
} Frame;

void wait_low_clk() {
    while (true) {
		int value = gpio_get(CLKIN);
		if (value == 0) {
			return;
		}
	}
}

void wait_high_clk() {
    while (true) {
		int value = gpio_get(CLKIN);
		if (value > 0) {
			return;
		}
	}
}


// Bit (0)
//    CLK
//     ___
//    |  |
// ---   ---
//
//   DIO
//
// ----------
//
// Bit (1)
//    CLK
//     ___
//    |  |
// ---   ---
//
//   DIO
//   ______
//  |     |
// -      --
//
// Frame end
//    CLK
//   ______
//  |     |
// -      --
//
//   DIO
//    ___
//   |  |
// --   ---
 typedef enum {
    BIT0 = 0,
    BIT1 = 1,
    FRAME_END,
} SCAN;


SCAN scan() {
    while (true) {
        int clk = gpio_get(CLKIN);
        int din = gpio_get(DIOIN);

		// Rising clock edge.
        if (clk == 1) {
			// If din == 1, then we know this is a valid bit (1).
			if (din == 1) {
				while (true) {
					int clk = gpio_get(CLKIN);

					// Falling clock edge.
					if (clk == 0) {
						return BIT1;
					}
				}
			}

			// If bit == 0, then either a valid bit (0) or a frame end, determine
			// based on which rising edge is hit first.
			while (true) {
				int clk = gpio_get(CLKIN);
        		int din = gpio_get(DIOIN);

				// Falling clock edge.
				if (clk == 0) {
					return BIT0;
				}

				// Rising din edge (while clock signal is high).
				if (din == 1) {
					return FRAME_END;
				}
			}
        }
    }
}

Frame read_frame() {
	Frame f = {0};
	int bit_count = 0;

	wait_low_clk(); // scan must be called on low clk
    while (true) {
		SCAN res = scan();

		int byte_idx = bit_count / 8;
		int bit_idx  = bit_count % 8;

		if (res == FRAME_END) {
			if (bit_count == 0 || bit_idx != 0) {
				// Something bad happened, skip this frame.
				f.length = 0;
				bit_count = 0;
				wait_low_clk();
				continue;
			}
			f.length = byte_idx;
			return f;
		}

		int bit = res == BIT1;
		f.data[byte_idx] |= (bit << bit_idx);
		bit_count++;

		// scan returned on falling clock edge.
    }
}

queue_t q;

void data_receive_core() {
	int i = 0;
    while (true) {
		Frame f = read_frame();


		if ((f.data[0] & 0b11000000) == 0b10000000) {
			int n = i%50;
			if (n < 10) {
				f.data[0] = 0b10001000;
			} else if (n < 20) {
				f.data[0] = 0b10001001;
			} else if (n < 30) {
				f.data[0] = 0b10001010;
			} else if (n < 40) {
				f.data[0] = 0b10001011;
			} else if (n < 50) {
				f.data[0] = 0b10001111;
			}
		}

		queue_add_blocking(&q, &f);
		i++;
    }
}

int main() {
    gpio_init(DIOIN);
    gpio_init(CLKIN);
    gpio_init(DIOOUT);
    gpio_init(CLKOUT);
    gpio_set_dir(DIOIN, GPIO_IN);
    gpio_set_dir(CLKIN, GPIO_IN);
    gpio_set_dir(DIOOUT,GPIO_OUT);
    gpio_set_dir(CLKOUT,GPIO_OUT);

    // gpio_init(PICO_DEFAULT_LED_PIN);
    // gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

	queue_init(&q, sizeof(Frame), 16);
	multicore_launch_core1(data_receive_core);

    while (true) {
		gpio_put(CLKOUT,1);
		sleep_us(3);
		gpio_put(DIOOUT, 1);

		Frame f;
		queue_remove_blocking(&q, &f);

		sleep_us(5);

		gpio_put(DIOOUT, 0);
		sleep_us(3);
		gpio_put(CLKOUT, 0);
		sleep_us(3);

		for (int i = 0; i < f.length; i++) {
			uint8_t val = f.data[i];
			for (int j = 0; j < 8; j++) {
				int bit = (val >> j) & 1;
				gpio_put(DIOOUT,bit);
				sleep_us(3);
				gpio_put(CLKOUT, 1);
				sleep_us(5);
				gpio_put(CLKOUT, 0);
				sleep_us(3);
			}
		}

		gpio_put(DIOOUT,0);
		sleep_us(5);
    }
}
