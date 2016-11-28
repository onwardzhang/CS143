<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->
    <title>CS143 Project 1c Yanming Zhang</title>

    <!-- Bootstrap -->
    <link href="bootstrap.min.css" rel="stylesheet">
    <link href="main.css" rel="stylesheet">
    <script src="jquery-3.1.1.min.js"></script>
  <script src="bootstrap.min.js"></script>
      </head>

  <body>

    <nav class="navbar navbar-inverse navbar-fixed-top">
      <div class="container-fluid">
        <div class="navbar-header navbar-defalt">
          <a class="navbar-brand" href="#">Movie Database Query System</a>
        </div>
        <ul class="nav navbar-nav">
      <li><a href="myhomepage.php">Home</a></li>
      <li class="dropdown">
        <a class="dropdown-toggle" data-toggle="dropdown" href="#">Add new content
        <span class="caret"></span></a>
        <ul class="dropdown-menu">
          <li><a href="Add_A_D.php">Add Actor/Director</a></li>
          <li><a href="Add_movie.php">Add Movie Information</a></li>
          <li><a href="Add_comment.php">Add Movie Comment</a></li>
          <li><a href="Add_M_A_R.php">Add Movie/Actor Relation</a></li>
          <li><a href="Add_M_D_R.php">Add Movie/Director Relation</a></li>
        </ul>
      <li class="dropdown">
        <a class="dropdown-toggle" data-toggle="dropdown" href="#">Browsering Content
        <span class="caret"></span></a>
        <ul class="dropdown-menu">
            <li><a href="Show_A.php">Show Actor Information</a></li>
            <li><a href="Show_M.php">Show Movie Information</a></li>
        </ul>
      </li>
      <li><a href="search.php">Search Interface</a></li> 
    </ul>
      </div>
    </nav>

    <?php
    $db = new mysqli('localhost', 'cs143', '', 'CS143');
    //$db = new mysqli('localhost', 'cs143', '', 'TEST');

  if($db->connect_errno > 0){
      die('Unable to connect to database [' . $db->connect_error . ']');
  }
  ?>

</body>
</html>


