void construct_frame(uint8_t *buffer, uint8_t address, uint8_t control)
{
    buffer[0] = FLAG;
    buffer[1] = address;
    buffer[2] = control;
    buffer[3] = BCC(buffer[1], buffer[2]);
    buffer[4] = FLAG;
}


int send_frame(int fd, uint8_t address, uint8_t control, command response)
{
    uint8_t *buffer = (uint8_t *)malloc(5);
    memset(buffer, 0, 5);
    int bytes;

    construct_frame(buffer, address, control);

    if ((bytes = send_message(fd, buffer, 5, response)) == -1)
    {
        free(buffer);
        return -1;
    }

    free(buffer);
    return bytes;
}
