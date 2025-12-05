<?php
session_start();
$registerError = $_SESSION['register_error'] ?? '';
unset($_SESSION['register_error']);
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
                <h2>Sign Up</h2>
                <?php if($registerError): ?>
                <p class="error-message"><?= $registerError ?></p>
                <?php endif; ?>

                <input type="text" name="name" placeholder="Nombre" required>
                <input type="text" name="email" placeholder="Correo" required>
                <input type="password" name="password" placeholder="Contraseña" required>
                <button type="submit" name="register">Sign Up</button>
                <p>¿Ya tienes una cuenta? <a href="login.php">Log In</a></p>
            </form>
        </div>

    </div> 
</body>
</html>