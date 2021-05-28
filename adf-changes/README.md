# ADF Changes

These attached files create a simple "Generic ESP32" board in the menuconfig and allow for running the ADF on a standard ESP32. Note the pin configuration for the I2S pins can be found in 'board_pins_config.c'. The WS, BCLK and Data In/Out pins can be changed to any valid pin. The MCLK pin has to be one of GPIO 0, 1 or 3. Pin 1 is the UART TX so thats not ideal, Pin 0 is not exposed on the board I was using so I chosen Pin 3

Simply copy the files to the esp-adf/components/audio_board directory and re-run 'idf.py menuconfig'

