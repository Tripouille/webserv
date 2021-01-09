#!/bin/bash
export GATEWAY_INTERFACE="CGI/1.1"
export SERVER_PROTOCOL="HTTP/1.1"
export SCRIPT_FILENAME="/Users/jgambard/webserv/cgitest/test.php"
export SCRIPT_NAME="test.php"
export REDIRECT_STATUS="200"

/Users/jgambard/.brew/bin/php-cgi test.php
exit 0