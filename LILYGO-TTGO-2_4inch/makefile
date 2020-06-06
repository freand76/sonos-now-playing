CLI=cli/arduino-cli
SKETCHNAME=SonosNowPlaying

all:

compile:
	$(CLI) compile --fqbn esp32:esp32:esp32 $(SKETCHNAME)

upload:
	$(CLI) upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32:"UploadSpeed=921600" $(SKETCHNAME)

install-libs:
	$(CLI) lib install TFT_eSPI

setup: cli/arduino-cli

cli/arduino-cli:
	wget --quiet --output-document arduino-cli.tar.gz https://downloads.arduino.cc/arduino-cli/arduino-cli_0.10.0_Linux_64bit.tar.gz
	mkdir -p cli
	cd cli && tar zxvf ../arduino-cli.tar.gz
	rm -rf arduino-cli.tar.gz

setup-clean:
	rm -rf cli

debug-terminal:
	picocom /dev/ttyUSB0 -b 115200

# * cli/arduino-cli config init
#
# * update /home/<your user>/.arduino15/arduino-cli.yaml
#
#   board_manager:
#     additional_urls:
#       - https://dl.espressif.com/dl/package_esp32_index.json
#   daemon:
#
# * cli/arduino-cli core update-index
#
# * cli/arduino-cli core install esp32:esp32
#
# * make install-libs
#
# * make compile
#
# * make upload
#
