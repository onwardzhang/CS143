<html>
<body>

<?php include 'myBase.php';?>
<br><br><br><br><br><br>

<div class="col-sm-9 col-sm-offset-3 col-md-5 col-md-offset-2 main">
  <h3>Add a new movie-director relation</h3>
  <form action="Add_M_D_R.php" method="POST">

   <div class="form-group">
     <label for="sel1">Movie title</label>
        <select class="form-control" name="title">
  			<?php
  				if(!($result = $db->query("SELECT title FROM Movie ORDER BY title ASC"))){
  				 	$errmsg = $db->error;
            		print "Query failed: $errmsg <br />";
            		exit(1);
  				} else {
					while($row = $result->fetch_row()){
						?>
						<OPTION> <?php echo $row[0];?> </OPTION>
						<?php }
						$result->free();}?>
        </select>
   </div>

   <div class="form-group">
     <label for="sel2">Director</label>
        <select class="form-control" name="director">
  			<?php
  				if(!($result2 = $db->query("SELECT CONCAT(first,' ', last) fullName FROM Director ORDER BY fullName ASC ", MYSQLI_USE_RESULT))){
  				 	$errmsg = $db->error;
            		print "Query failed: $errmsg <br />";
            		exit(1);
  				} else {
					while($row2 = $result2->fetch_row()){
						?>
						<OPTION> <?php echo $row2[0];?> </OPTION>
						<?php }
						$result2->free();}
			?>
        </select>
   </div>

    <button type="submit" class="btn btn-default">Add!</button>
    </form>
</div>

 <?php
    //if (isset($_POST[title], $_POST[director])){
      if ($_POST[title] && $_POST[director]){
      // echo $_POST[title];
      // echo '<br>';
      // echo $_POST[director];
      // echo '<br>';

	  $directorFullname = explode(" ", $_POST[director]);

        if (!($didResult = $db->query("SELECT id From Director WHERE first ='$directorFullname[0]' AND last = '$directorFullname[1]' "))){
            $errmsg = $db->error;
            print "Query failed: $errmsg <br />";
            exit(1);
        } else {
          $didArray = mysqli_fetch_assoc($didResult);
          $did =  $didArray['id'];
          //echo "$did<br />";
          $didResult->free();
        }

        if (!($midResult = $db->query("SELECT id From Movie WHERE title = '$_POST[title]' "))){
            $errmsg = $db->error;
            print "Query failed: $errmsg <br />";
            exit(1);
        } else {
          $midArray = mysqli_fetch_assoc($midResult);
          $mid =  $midArray['id'];
          //echo "$mid<br />";
          $midResult->free();
        }
      $insert = "INSERT INTO MovieDirector(mid, did) VALUES($mid, $did)";
      if (!$db->query($insert)){
        $errmsg = $db->error;
        print "Query failed: $errmsg <br>";
        exit(1);
      } else{
        echo '<h4>Insert successfully!<br></h4>';
      }
    }
?>

<?php $db->close();?>
</body>
</html>