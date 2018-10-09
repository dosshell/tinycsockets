﻿/*
 * Copyright 2018 Markus Lindelöw
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <tinycsocket.h>

#include <stdio.h>
#include <stdlib.h>

int show_error(const char* error_text)
{
    fprintf(stderr, "%s", error_text);
    return -1;
}

int main()
{
    if (tcs_lib_init() != TINYCSOCKET_SUCCESS)
        return show_error("Could not init tinycsockets");

    tcs_socket listen_socket = TINYCSOCKET_NULLSOCKET;
    tcs_socket child_socket = TINYCSOCKET_NULLSOCKET;

    struct tcs_addrinfo hints = { 0 };

    hints.ai_family = TINYCSOCKET_AF_INET;
    hints.ai_protocol = TINYCSOCKET_IPPROTO_TCP;
    hints.ai_socktype = TINYCSOCKET_SOCK_STREAM;
    hints.ai_flags = TINYCSOCKET_AI_PASSIVE;

    struct tcs_addrinfo* listen_addressinfo = NULL;
    if (tcs_getaddrinfo(NULL, "1212", &hints, &listen_addressinfo) != TINYCSOCKET_SUCCESS)
        return show_error("Could not resolve listen address");

    if (tcs_create(&listen_socket, listen_addressinfo->ai_family, listen_addressinfo->ai_socktype, listen_addressinfo->ai_protocol) != TINYCSOCKET_SUCCESS)
        return show_error("Could not create a listen socket");

    if (tcs_bind(listen_socket, listen_addressinfo->ai_addr, listen_addressinfo->ai_addrlen) != TINYCSOCKET_SUCCESS)
        return show_error("Could not bind to listen address");

    if (tcs_freeaddrinfo(&listen_addressinfo) != TINYCSOCKET_SUCCESS)
        return show_error("Could not free address info");

    if (tcs_listen(listen_socket, TINYCSOCKET_BACKLOG_SOMAXCONN) != TINYCSOCKET_SUCCESS)
        return show_error("Could not listen");

    if (tcs_accept(listen_socket, &child_socket, NULL, NULL) != TINYCSOCKET_SUCCESS)
        return show_error("Could not accept socket");

    if (tcs_close(&listen_socket) != TINYCSOCKET_SUCCESS)
        return show_error("Could not close listen socket");

    uint8_t recv_buffer[1024];
    size_t bytes_recieved = 0;
    if (tcs_recv(child_socket, recv_buffer, sizeof(recv_buffer) - sizeof('\0'), 0, &bytes_recieved) != TINYCSOCKET_SUCCESS)
        return show_error("Could not recieve data from client");

    recv_buffer[bytes_recieved] = '\0';
    printf("recieved: %s\n", recv_buffer);

    char msg[] = "I here you loud and clear\n";
    if (tcs_send(child_socket, msg, sizeof(msg), 0, NULL) != TINYCSOCKET_SUCCESS)
        return show_error("Could not send reply message");

    if (tcs_shutdown(child_socket, TINYCSOCKET_SD_BOTH) != TINYCSOCKET_SUCCESS)
        return show_error("Could not shutdown socket");

    if (tcs_close(&child_socket) != TINYCSOCKET_SUCCESS)
        return show_error("Could not close socket");

    if (tcs_lib_free() != TINYCSOCKET_SUCCESS)
        return show_error("Could not free tinycsockets");
}