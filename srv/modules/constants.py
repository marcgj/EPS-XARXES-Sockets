from enum import Enum


class Types(Enum):
    REG_REQ = 0xa0
    REG_ACK = 0xa1
    REG_NACK = 0xa2
    REG_REJ = 0xa3
    REG_INFO = 0xa4
    INFO_ACK = 0xa5
    INFO_NACK = 0xa6
    INFO_REJ = 0xa7

    ALIVE = 0xb0
    ALIVE_NACK = 0xb1
    ALIVE_REJ = 0xb2

    SEND_DATA = 0xc0
    DATA_ACK = 0xc1
    DATA_NACK = 0xc2
    DATA_REJ = 0xc3
    SET_DATA = 0xc4
    GET_DATA = 0xc5


def enum_from_value(enum, value):
    for elem in enum:
        if elem.value == value:
            return elem


ZEROS = "0000000000"


class Status(Enum):
    DISCONNECTED = 0xf0
    NOT_REGISTERED = 0xf1
    WAIT_ACK_REG = 0xf2
    WAIT_INFO = 0xf3
    WAIT_ACK_INFO = 0xf4
    REGISTERED = 0xf5
    SEND_ALIVE = 0xf6

