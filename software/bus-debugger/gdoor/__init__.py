def tohex(word):
    return "{0:#0{1}x}".format(word,4).upper().replace("X", "x")

class GDOOR():
    words = None

    meanings = {
        9: {
            "len": 3,
            "name": "Destination",
            "type": "uint",
        },
        8: {
            "len": 1,
            "name": "HW-Type",
            "type": "ENUM",
            "choices": {
                "0xA1": "Indoor",
                "0xA0": "Outdoor",
                "0xA3": "Controller"
            }
        },
        6: {
            "len": 2,
            "name": "Parameter?/No. BUTTON_RING",
            "type": "uint",
        },
        3: {
            "len": 3,
            "name": "Source",
            "type": "uint",
        },
        2: {
            "len": 1,
            "name": "Action",
            "type": "ENUM",
            "choices": {
                0x42: "BUTTON",
                0x41: "BUTTON_LIGHT",
                0x31: "DOOR_OPEN",
                0x28: "VIDEO_REQUEST",
                0x21: "AUDIO_REQUEST",
                0x20: "AUDIO_VIDEO_END",
                0x13: "BUTTON_FLOOR",
                0x12: "CALL_INTERNAL",
                0x11: "BUTTON_RING",
                0x0F: "CTRL_DOOROPENER_ACK",
                0x08: "CTRL_RESET",
                0x05: "CTRL_DOORSTATION_ACK",
                0x04: "CTRL_BUTTONS_TRAINING_START",
                0x03: "CTRL_DOOROPENER_TRAINING_START",
                0x02: "CTRL_DOOROPENER_TRAINING_STOP",
                0x01: "CTRL_PROGRAMMING_START",
                0x00: "CTRL_PROGRAMMING_STOP"
            },
        },
        1: {
            "len": 1,
            "name": "State ??",
            "type": "ENUM",
            "choices": {
                "0xC0": "Ack? (Länge 2)",
                "0x10": "Length 1?",
                "0x00": "Length 2"
            }
        },
        0: {
            "len": 1,
            "name": "Length ??",
            "type": "uint",
        },
    }

    @classmethod
    def checksum(cls, words):
        chksum = 0
        for word in words:
            chksum = chksum + int(word)&0xFF
        return chksum&0xFF

    def __init__(self, string):
        self.words = bytearray.fromhex(string)

    def __str__(self):
        string = ""
        details = ""
        if self.words is not None:
            index = 0
            for word in self.words:
                string += tohex(word) + " "
                if index in self.meanings:
                    elem = self.meanings[index]
                    if index + elem["len"] <= len(self.words):
                        if elem["type"] == "ENUM":
                            name = elem["name"]
                            value = ""
                            for i in range(index, index+elem["len"]):
                                value += tohex(self.words[i]) + " "

                            value = value[:-1]
                            if value in elem["choices"]:
                                value += " " + elem["choices"][value]
                            else:
                                value += " UNKOWN"
                            details += f"\n{name}: {value}"

                        if elem["type"] == "uint":
                            name = elem["name"]
                            value = ""
                            for i in range(index, index+elem["len"]):
                                value += tohex(self.words[i]) + " "
                            details += f"\n{name}: {value}"
                index += 1
            checksum = "{0:#0{1}x}".format(self.words[-1], 4)
            details += f"\nCheckSum: {checksum} / Checked: {self.checksum(self.words)}"
        
        return string + details