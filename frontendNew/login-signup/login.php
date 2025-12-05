<?php
session_start();
$loginError = $_SESSION['login_error'] ?? '';
unset($_SESSION['login_error']);
?>


<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login and Register</title>
    <link rel="stylesheet" href="login-signup-style.css">
</head>
<body>
    <div class="container">
        <img src="../resources/ffflux.svg" alt="Gradient Decoration" class="gradient">
        <div class="form-box" id="login-form">
            <form action="login-signup.php" method="post">
                <h2>Log In</h2>
                <?php if($loginError): ?>
                <p class="error-message"><?= $loginError ?></p>
                <?php endif; ?>

                <input type="text" name="email" placeholder="Correo" required>
                <input type="password" name="password" placeholder="Contraseña" required>
                <button type="submit" name="login" >Log In</button>
                <p>¿No tienes una cuenta? <a href="signup.php">Sign Up</a></p>
        </div>
    </div> 
</body>
</html>