<!DOCTYPE html>
<html lang="fr">
    <head>
        <meta charset="utf-8" />
        <title>Test PHP</title>
    </head>

    <body>
<?php	if (isset($_GET['var']) AND $_GET['var'] == '1')
		{ ?>
			<p>Bonjour ! (1)</p>
<?php	}
		else
		{ ?>
			<p>Bonjour ! (pas 1)</p>
<?php	} ?>
	</body>
</html>
