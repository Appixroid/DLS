# DLS

The Distant Log System (DLS) is a server that accept DLS request to create distant log file.
This repository contains a server and an exemple client for testing.

## Usage

Use the **makefile** to compile the both server and client application.

`make`

The server can be launched with the *Start_DLS_Server.sh* shell file, and the client with the *open_client.sh* shell file.

## Requesting Protocol

The actual version is *DLS 1.0*, and the length of a request is **308 bytes**.

<table>
    <tr>
      <td></td>
      <td>308 Bytes</td>
      <td></td>
    </tr>
    <tr>
        <th>8 Bytes</th>
        <th>44 Bytes</th>
        <th>256 Bytes</th>
    </tr>
    <tr>
        <td>Version</td>
        <td>Request Sender</td>
        <td>Log Text</td>
    </tr>
</table>
