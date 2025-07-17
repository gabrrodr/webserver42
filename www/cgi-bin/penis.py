#!/usr/bin/env python3

import os
import cgi
import cgitb
import sys
cgitb.enable()

print("Content-Type: text/html\n")

form = cgi.FieldStorage(fp=sys.stdin, environ=os.environ, keep_blank_values=True)

name = form.getvalue("name", "(no name)")
email = form.getvalue("email", "(no email)")

print(f"""
<html>
<head><title>CGI POST Response</title></head>
<body>
  <h1>POST Data Received</h1>
  <p>Name: {name}</p>
  <p>Email: {email}</p>
</body>
</html>
""")
