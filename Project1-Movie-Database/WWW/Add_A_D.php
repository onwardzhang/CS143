<html>
<body>

<?php include 'myBase.php';?>
<br><br><br><br><br><br>

<div class="col-sm-9 col-sm-offset-3 col-md-5 col-md-offset-2 main">
  <h3>Add a new Actor/Director</h3>
  <form action="Add_A_D.php" method="POST">
    <label class="radio-inline">
      <input type="radio" checked="checked" name="identity" value="Actor"/>Actor
    </label>
    <label class="radio-inline">
      <input type="radio" name="identity" value="Director"/>Director
    </label>

        <div class="form-group">
          <label for="first_name">First Name</label>
          <input type="text" class="form-control" placeholder="Text input"  name="fName"/>
        </div>

        <div class="form-group">
          <label for="last_name">Last Name</label>
          <input type="text" class="form-control" placeholder="Text input" name="lName"/>
        </div>

        <label class="radio-inline">
            <input type="radio" name="sex" checked="checked" value="male">Male
        </label>
        <label class="radio-inline">
            <input type="radio" name="sex" value="female">Female
        </label>

        <div class="form-group">
          <label for="DOB">Date of Birth</label>
          <input type="text" class="form-control" placeholder="Text input" name="dateB"/> 
        </div>

        <div class="form-group">
          <label for="DOD">Date of Die</label>
          <input type="text" class="form-control" placeholder="Blank if still alive" name="dateD"/>
        </div>
        <button type="submit" class="btn btn-default">Add!</button>
    </form>




</div>

<?php
	//todo: fix the bug of can query even some attribute is '';
	//if (isset($_POST["fName"], $_POST["lName"],$_POST["dateB"])){
  if ($_POST["fName"] && $_POST["lName"] && $_POST["dateB"]){
	// if ($_POST[fname] == '' || $_POST[lname] == '' || $_POST[dateB] == ''){//if some parts of the form are incomplete, don't query,give warning!
	// 		echo 'Please complete the form!';
	// } else{
		if($_POST["dateD"]!=''){//应该用！=‘’判断！
			$dateOfDeath = $_POST["dateD"];
		} else {
			//echo 'dod is null<br>';
			$dateOfDeath = "NULL";
		}
		//echo $dateOfDeath;

		if (!($personIDResult = $db->query("SELECT id From MaxPersonID"))){
  			$errmsg = $db->error;
    		print "Query failed: $errmsg <br />";
    		exit(1);
  		} else{
				$personID = mysqli_fetch_assoc($personIDResult);
  			$newMaxPersonID =  $personID['id']+1;
  			//echo "$newMaxPersonID<br />";
        $personIDResult->free();
  		}
  		//echo "$newMaxPersonID<br />";

  		if($_POST[identity]=='Actor'){
  			$query = "INSERT INTO $_POST[identity] (id,last,first,sex,dob,dod) VALUES($newMaxPersonID,'$_POST[lName]','$_POST[fName]','$_POST[sex]', '$_POST[dateB]','$dateOfDeath')";
  		} else{
  			$query = "INSERT INTO $_POST[identity] (id,last,first,dob,dod) VALUES($newMaxPersonID,'$_POST[lName]','$_POST[fName]', '$_POST[dateB]','$dateOfDeath')";//Director table doesn't have 'sex' attribute.!! maybe can use invisible for label sex when choose director.
  		}
		    
		    //echo $query;

		if (!$db->query($query)){
			$errmsg = $db->error;
    	print "Query failed: $errmsg <br>";
    	exit(1);
		} else{
			echo '<h4>Add successfully!</h4>';
			$update = "UPDATE MaxPersonID SET id=$newMaxPersonID";
			if (!$db->query($update)){
				$errmsg = $db->error;
    		print "Query failed: $errmsg <br />";
    		exit(1);
			} else{
				echo '<h4>Update MaxPersonID successfully!</h4>';
			}
		}
	} else {
    if ($_POST[identity]|| $_POST[sex] || $_POST["fName"] || $_POST["lName"] || $_POST["dateB"]){
      echo '<h4>Failed! Some information is empty. <br>Please complete all the information! </h4>';
    }
  } 
	?> 

<?php $db->close();?>
</body>
</html>