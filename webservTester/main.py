import client
import os

class colors:
	GREEN = '\033[92m'
	RED = '\033[91m'
	RESET = '\033[0m'

def check(name, success):
	print(colors.RESET, name + ": ", colors.GREEN + "OK " if success else colors.RED + "KO ", colors.RESET, end='', sep='')

def test(expectedStatus, method, uri, body, headers, expectedHeaders, expectedBody):
	webserv = client.HTTPConnection('localhost:9999')
	print(colors.RESET, '{0:-<130}'.format("\n{} {} {}".format(method, uri, headers)), end='')
	webservAnswer = doRequest(webserv, method, uri, body, headers)
	check("Status", expectedStatus == webservAnswer.status)
	headerSuccess = True
	if expectedHeaders:
		expectedHeaders = {k.lower(): v for k, v in expectedHeaders.items()}
		headers = {k.lower(): v for k, v in dict(webservAnswer.getheaders()).items()}
		for key, value in expectedHeaders.items():
			if key not in headers or headers[key] != value:
				headerSuccess = False
				break
	check("Headers", headerSuccess)
	body = webservAnswer.read().decode("utf-8")
	check("Body", not expectedBody or body == expectedBody)
	webserv.close()

def doRequest(webserv, method, path, body, headers):
	webserv.request(method, path, body, headers)
	webservAnswer = webserv.getresponse()
	return webservAnswer

def fileTest(expectedStatus, method, uri, body, headers, expectedHeaders, expectedBody, file):
	if os.path.exists(file): os.remove(file)
	test(expectedStatus, method, uri, body, headers, expectedHeaders, expectedBody)
	check("File", os.path.exists(file))

test(200, 'GET', '/', None, {"Host": "localhost"}, None, None)
test(400, 'GET', '/', None, {"Host": ""}, None, None)
test(200, 'GET', '/languages', None, {"Host": "localhost"}, None, None)
test(200, 'GET', '/languages/language.html', None, {"Host": "localhost"}, None, None)
test(200, 'GET', '/languages/language.html', None, {"Host": "localhost", "Accept-Language": "fr"}, {"Content-Location" : "/languages/language.html.fr-CA"}, None)
test(200, 'GET', '/languages/language.html', None, {"Host": "localhost", "Accept-Language": "fr-FR"}, {"Content-Location" : "/languages/language.html.fr-FR"}, None)
test(200, 'GET', '/languages/language.html', None, {"Host": "localhost", "Accept-Language": "fr-FR;q=0.5, en"}, {"Content-Location" : "/languages/language.html.en"}, None)
test(200, 'GET', '/directory/nop', None, {"Host": "localhost"}, None, "youpi.bad_extension file in YoupiBanane/nop/ (in webservTester)")
test(404, 'GET', '/directory/nopo', None, {"Host": "localhost"}, None, None)
#test(403, 'GET', '/forbiddenFile', None, {"Host": "localhost"}, {"Content-Length": "9"}, "Error 403")
test(401, 'GET', '/private', None, {"Host": "localhost"}, None, "401 Unauthorized")
test(200, 'GET', '/private', None, {"Host": "localhost", "Authorization": "Basic dXNlcjEyMzpwYXNzd29yZDEyMw=="}, None, None) #user123 password123
test(200, 'GET', '/prout.prout', None, {"Host": "localhost"}, None, None) #devrait pas tourner a l'infini
print()

test(200, 'POST', '/post_test/secret.php', "mot_de_passe=kangourou", {"Host": "localhost", "Content-type": "application/x-www-form-urlencoded", "Content-Length": "22"}, {"Content-Length": "2", "content-type": "text/html; charset=UTF-8"}, "42")
test(200, 'POST', '/post_test/secret.php', "", {"Host": "localhost", "Content-type": "application/x-www-form-urlencoded", "Content-Length": "0"}, {"Content-Length": "2"}, "43")
test(405, 'POST', '/', None, {"Host": "localhost"}, {"Allow": "GET"}, None)
fileTest(201, 'POST', '/post_test/createdFile', None, {"Host": "localhost"}, None, None, "www/put_saves/createdFile")
print()

fileTest(201, 'PUT', '/put_test/createdFile', None, {"Host": "localhost"}, None, None, "www/put_saves/createdFile")
test(204, 'PUT', '/put_test/test_put_save', "coucou", {"Host": "localhost"}, None, None)
print()

test(501, 'GETO', '/', None, {"Host": "localhost"}, None, None)
print(colors.RESET)


test(200, 'GET', '/index.php', None, {"Host": "localhost"}, None, None)
# Stress test
for i in range(1000):
	webserv = client.HTTPConnection('localhost:9999')
	webservAnswer = doRequest(webserv, 'GET', '/index.php', None, {"Host": "localhost"})
	webservAnswer.read()
	webserv.close()

