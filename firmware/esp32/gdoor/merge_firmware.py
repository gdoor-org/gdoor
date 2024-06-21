# see https://github.com/platformio/platform-espressif32/issues/1078#issuecomment-1636793463
Import("env")
import json
import os

APP_BIN = "$BUILD_DIR/${PROGNAME}.bin"
WEB_DIR = ".pio/build/GDOOR_ESP32MINI/web"
MERGED_BIN = WEB_DIR + "/${PROGNAME}_merged.bin"
BOARD_CONFIG = env.BoardConfig()
MANIFEST_JSON = "manifest.json"
MANIFEST_JSON_RESULT = WEB_DIR + "/manifest.json"

if not os.path.exists(WEB_DIR):
    os.makedirs(WEB_DIR)

def merge_bin(source, target, env):
    # The list contains all extra images (bootloader, partitions, eboot) and
    # the final application binary
    flash_images = env.Flatten(env.get("FLASH_EXTRA_IMAGES", [])) + ["$ESP32_APP_OFFSET", APP_BIN]

    # Run esptool to merge images into a single binary
    env.Execute(
        " ".join(
            [
                "$PYTHONEXE",
                "$OBJCOPY",
                "--chip",
                BOARD_CONFIG.get("build.mcu", "esp32"),
                "merge_bin",
                "--fill-flash-size",
                BOARD_CONFIG.get("upload.flash_size", "4MB"),
                "-o",
                MERGED_BIN,
            ]
            + flash_images
        )
    )

    # build web flasher manifest
    with open(MANIFEST_JSON, 'r', encoding='utf-8') as json_file:
        manifest = json.load(json_file)
        manifest["version"] = os.environ.get("GDOOR_VERSION", "dev")

    with open(MANIFEST_JSON_RESULT, 'w', encoding='utf-8') as json_file:
        json.dump(manifest, json_file)

# Add a post action that runs esptoolpy to merge available flash images
env.AddPostAction(APP_BIN, merge_bin)