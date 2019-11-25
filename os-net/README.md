# Sockets simple

There are server-client simple application based on ```Unix``` sockets.

## Server:
 * Accepts address with it listens
 * Starts, binds to address and waits for connections
 * Accepts connection, executes server logic and returns to waiting

## Client:
 * Accepts address, connects, executes client logic and terminates.