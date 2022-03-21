const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<!-- Required meta tags -->
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
<!-- Bootstrap CSS -->
<link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css" integrity="sha384-9aIt2nRpC12Uk9gS9baDl411NQApFmC26EwAOH8WgZl5MYYxFfc+NcPb1dKGj7Sk" crossorigin="anonymous">
</head>
<body>
<h1>LED Clock Board - L&R TECH</h1>
<h4>Send a message to the display!<h4>
<form id='message-form' class='form' method='GET' action='/message'>
<input type='text' class='form-control' required='required' name='m' maxlength='30' placeholder="Enter Message for LED board Here!">
<button type='submit' class='btn btn-primary'>Send</button>
</form>
</body>
</html>
)=====";
