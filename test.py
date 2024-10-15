#!/usr/bin/env python3
import sys
import datetime

# Prepare the HTML content
html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Accurate CGI Script</title>
</head>
<body>
    <h1>Hello from Python CGI!</h1>
    <p>The current date and time is: {datetime.datetime.now()}</p>
</body>
</html>
"""

# Convert content to bytes for accurate length calculation
content_bytes = html_content.encode('utf-8')
content_length = len(content_bytes)
# while True:
#     print("test\n")
# Print headers with proper line endings
sys.stdout.buffer.write(b"Content-Type: text/html; charset=utf-8\r\n")
sys.stdout.buffer.write(f"Content-Length: {content_length}\r\n".encode('utf-8'))
sys.stdout.buffer.write(b"\r\n")  # Empty line to separate headers from content

# Print the content
sys.stdout.buffer.write(content_bytes)