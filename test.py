# #!/usr/bin/env python3
# import sys
# import datetime

# # Prepare the HTML content
# html_content = f"""<!DOCTYPE html>
# <html lang="en">
# <head>
#     <meta charset="UTF-8">
#     <title>Accurate CGI Script</title>
# </head>
# <body>
#     <h1>Hello from Python CGI!</h1>
#     <p>The current date and time is: {datetime.datetime.now()}</p>
# </body>
# </html>
# """

# # Convert content to bytes for accurate length calculation
# content_bytes = html_content.encode('utf-8')
# content_length = len(content_bytes)
# # while True:
# #     print("test\n")
# # Print headers with proper line endings
# sys.stdout.buffer.write(b"Content-Type: text/html; charset=utf-8\r\n")
# sys.stdout.buffer.write(f"Content-Length: {content_length}\r\n".encode('utf-8'))
# sys.stdout.buffer.write(b"\r\n")  # Empty line to separate headers from content

# # Print the content
# sys.stdout.buffer.write(content_bytes)


#!/usr/bin/env python
# Importing the 'cgi' module
import cgi

# Send an HTTP header indicating the content type as HTML
print("Content-type: text/html\n\n")

# Start an HTML document with center-aligned content
print("<html><body style='text-align:center;'>")

# Display a green heading with text "GeeksforGeeks"
print("<h1 style='color: green;'>GeeksforGeeks</h1>")

# Parse form data submitted via the CGI script
form = cgi.FieldStorage()

# Check if the "name" field is present in the form data
if form.getvalue("name"):
    # If present, retrieve the value and display a personalized greeting
    name = form.getvalue("name")
    print("<h2>Hello, " + name + "!</h2>")
    print("<p>Thank you for using our script.</p>")

# Check if the "happy" checkbox is selected
if form.getvalue("happy"):
    # If selected, display a message with a happy emoji
    print("<p>Yayy! We're happy too! ????</p>")

# Check if the "sad" checkbox is selected
if form.getvalue("sad"):
    # If selected, display a message with a sad emoji
    print("<p>Oh no! Why are you sad? ????</p>")

# Close the HTML document
print("</body></html>")
