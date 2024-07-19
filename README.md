# IMAP_Email_Client
This project is an IMAP Email Client written in C. It allows users to connect to an IMAP server, log in with their credentials, select a folder, and perform various email-related commands such as retrieving, parsing, and handling MIME content of emails.

Features
Retrieve Emails: Fetch the complete content of an email.
Parse Emails: Extract and display specific fields (From, To, Date, Subject) from an email's header.
Handle MIME: Process MIME-encoded emails and extract content based on boundaries.

Prerequisites
C Compiler (e.g., gcc)
Standard C libraries: stdio.h, stdlib.h, string.h, unistd.h, netdb.h, sys/socket.h, netinet/in.h, arpa/inet.h, ctype.h

Usage
Run the compiled program with the appropriate flags:
./fetchmail -u <username> -p <password> -f <folder> -n <message_number> -t <tls> <command> <server_name>

Flags and Arguments
-u <username>: Specify the username for login.
-p <password>: Specify the password for login.
-f <folder>: Specify the folder to select (default is INBOX).
-n <message_number>: Specify the message number to act upon.
-t <tls>: Use TLS for connection (optional).
<command>: The command to execute (retrieve, parse, mime).
<server_name>: The IMAP server address.

Contributing
Contributions are welcome. Please create a pull request or submit an issue if you have suggestions or bug reports.
