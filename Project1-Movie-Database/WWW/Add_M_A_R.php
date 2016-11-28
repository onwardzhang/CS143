<html>
<body>

<?php include 'myBase.php';?>
<br><br><br><br><br><br>

<div class="col-sm-9 col-sm-offset-3 col-md-5 col-md-offset-2 main">
  <h3>Add a new movie-actor relation</h3>
  <form action="Add_M_A_R.php" method="POST">

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
     <label for="sel2">Actor</label>
        <select class="form-control" name="actor">
  			<?php
  				if(!($result2 = $db->query("SELECT CONCAT(first,' ', last) fullName FROM Actor ORDER BY fullName ASC", MYSQLI_USE_RESULT))){
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

     <div class="form-group">
          <label for="Role">Role</label>
          <input type="text" class="form-control" placeholder="Text input"  name="role"/>
        </div>

    <button type="submit" class="btn btn-default">Add!</button>
    </form>
</div>

 <?php
    if ($_POST[title] && $_POST[actor] && $_POST[role]){

	  $actorFullname = explode(" ", $_POST[actor]);

        if (!($aidResult = $db->query("SELECT id From Actor WHERE first ='$actorFullname[0]' AND last = '$actorFullname[1]' "))){
            $errmsg = $db->error;
            print "Query failed: $errmsg <br />";
            exit(1);
        } else {
          $aidArray = mysqli_fetch_assoc($aidResult);
          $aid =  $aidArray['id'];
          //echo "$aid<br />";
          $aidResult->free();
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
      $insert = "INSERT INTO MovieActor(mid, aid, role) VALUES($mid, $aid, '$_POST[role]')";
      if (!$db->query($insert)){
        $errmsg = $db->error;
        print "Query failed: $errmsg <br>";
        exit(1);
      } else{
        echo '<h4>Insert successfully!<br></h4>';
      }
    } else {
      if ($_POST[title] || $_POST[actor] || $_POST[role]){
         echo '<h4>Failed! Some information is empty. <br>Please complete all the information! </h4>';
      }
    }
?>

<?php $db->close();?>
</body>
</html>