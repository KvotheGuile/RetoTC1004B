<?php
session_start();
require_once 'config.php';

// SIGN UP
if (isset($_POST['register'])) {
    $name = $_POST['name'];
    $email = $_POST['email'];
    $password = password_hash($_POST['password'], PASSWORD_DEFAULT);

    $checkEmail = $conn->query("SELECT email FROM users WHERE email = '$email'");
    if ($checkEmail->num_rows > 0) {
        $_SESSION['register_error'] = 'Este correo ya está registrado.';
        header("Location: signup.php");
        exit();
    } else {
        $conn->query("INSERT INTO users (name, email, pass, hospital_id) VALUES ('$name', '$email', '$password', '1')");
        $_SESSION['success_message'] = '¡Sign Up completado! Puedes hacer Log In.';
        header("Location: login.php");
        exit();
    }
}

// LOG IN
if (isset($_POST['login'])) {
    $email = $_POST['email'];
    //$password = password_hash($_POST['password'], PASSWORD_DEFAULT);
    $password = $_POST['password'];

    $checkEmail = $conn->query("SELECT name, email, pass FROM users WHERE email = '$email'");
    if ($checkEmail->num_rows > 0) {
        $user = $checkEmail->fetch_assoc();
        if (password_verify($password, $user['pass'])) {
        //if ($password == $user['pass']) {
            $_SESSION['name'] = $user['name'];
            $_SESSION['email'] = $user['email'];
            header("Location: ../dashboard/dashboard.html");
            exit();
        }
    }

    $_SESSION['login_error'] = 'Correo o contraseña incorrectos';
    header("Location: login.php");
    exit();
}
?>


?>
