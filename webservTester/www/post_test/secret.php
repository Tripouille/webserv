<?php
if (isset($_POST['mot_de_passe']) AND $_POST['mot_de_passe'] ==  "kangourou") // Si le mot de passe est bon
    echo '42';
else // Sinon, on affiche un message d'erreur
    echo '43';
?>