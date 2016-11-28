<html>
<body>

<?php include 'myBase.php';?>
<br><br><br><br><br><br>

<div class="col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2 main">
<!-- <h3> <?php //echo "$_GET[mid]'s";?> detail information </h3> -->

 <div class="page-header">
  <h3> Movie information(detail information, director, cast members and its reviews) </h3>
</div>

	  <h4>Movie detail information: </h4>
<?php
	$id = $_GET[id];

	if (!($rs = $db->query("SELECT * FROM Movie WHERE id = $id"))){
		$errmsg = $db->error;
    	print "Query failed: $errmsg <br />";
    	exit(1);
		}

	//table
	echo '<table border=2 cellspacing=5 cellpadding=5>';
	//header
	echo '<tr>';
	while ($column = $rs->fetch_field()) { 
        echo "<th>$column->name</th>";
	}
	echo '</tr>';

    //data
	while($row = $rs->fetch_array(MYSQLI_NUM)) {
		$i = 1;
		echo '<tr>';
		?>

		<td><a href =Actor_Info.php?id=<?php echo $row[0]; ?> ><?php echo $row[0]; ?></a></td>

		<?php
		while($i < $rs->field_count){
			if($row[$i] != null){
				echo "<td>$row[$i]</td>";
			}
			else{
				echo '<td>Unknown</td>';
			}
			$i = $i + 1;
		}
		echo '</tr>';
	}
	echo '</table>';
	//echo'<br><br>   <div class="page-header"><h4>Cast members: </h4></div>';
	echo'<br><br><h4>Genres: </h4>';
	$rs->free();
//--------------------------------------------
	if (!($rs4 = $db->query("SELECT genre FROM MovieGenre WHERE mid = $id"))){
		$errmsg4 = $db->error;
    	print "Query failed: $errmsg4 <br />";
    	exit(1);
		}

	//table
	echo '<table border=2 cellspacing=5 cellpadding=5>';
	//header
	echo '<tr>';
	while ($column4 = $rs4->fetch_field()) { 
        echo "<th>$column4->name</th>";
	}
	echo '</tr>';

    //data
	while($row4 = $rs4->fetch_array(MYSQLI_NUM)) {
		$i = 0;
		echo '<tr>';
		while($i < $rs4->field_count){
			if($row4[$i] != null){
				echo "<td>$row4[$i]</td>";
			}
			else{
				echo '<td>Unknown</td>';
			}
			$i = $i + 1;
		}
		echo '</tr>';
	}
	echo '</table>';
	//echo'<br><br>   <div class="page-header"><h4>Cast members: </h4></div>';
	echo'<br><br><h4>Director: </h4>';
	$rs4->free();
//--------------------------------------------

	if (!($rs1 = $db->query("SELECT id, CONCAT(first,' ', last) fullName, dob, dod FROM Director D WHERE D.id IN (SELECT did FROM MovieDirector WHERE mid = $id)"))){
		$errmsg1 = $db->error;
    	print "Query failed: $errmsg1 <br />";
    	exit(1);
		}

	//table
	echo '<table border=2 cellspacing=5 cellpadding=5>';
	//header
	echo '<tr>';
	while ($column1 = $rs1->fetch_field()) { 
        echo "<th>$column1->name</th>";
	}
	echo '</tr>';

    //data
	while($row1 = $rs1->fetch_array(MYSQLI_NUM)) {
		$i = 0;
		echo '<tr>';
		while($i < $rs1->field_count){
			if($row1[$i] != null){
				echo "<td>$row1[$i]</td>";
			}
			else{
				echo '<td>Unknown</td>';
			}
			$i = $i + 1;
		}
		echo '</tr>';
	}
	echo '</table>';
	//echo'<br><br>   <div class="page-header"><h4>Cast members: </h4></div>';
	echo'<br><br><h4>Cast members: </h4>';
	$rs1->free();


	if (!($rs2 = $db->query("SELECT A.id, CONCAT(A.first,' ', A.last) fullName, MA.role FROM Actor A, MovieActor MA WHERE MA.mid = $id AND MA.aid = A.id"))){
		$errmsg2 = $db->error;
    	print "Query failed: $errmsg2 <br />";
    	exit(1);
		}

	//table
	echo '<table border=2 cellspacing=2 cellpadding=5>';
	//header
	echo '<tr>';
	while ($column2 = $rs2->fetch_field()) { 
        echo "<th>$column2->name</th>";
	}
	echo '</tr>';

    //data
	while($row2 = $rs2->fetch_array(MYSQLI_NUM)) {
		$j = 1;
		echo '<tr>';
		?>

		<td><a href =Actor_Info.php?id=<?php echo $row2[0]; ?> ><?php echo $row2[0]; ?></a></td>

		<?php
		while($j < $rs2->field_count){
			if($row2[$j] != null){
				echo "<td>$row2[$j]</td>";
			}
			else{
				echo '<td>Unknown</td>';
			}
			$j = $j + 1;
		}
		echo '</tr>';
	}
	echo '</table>';
	//echo'<br><br><div class="page-header"><h4>Reviews of the movie: </h4></div>';
	echo'<br><br><h4>Reviews of the movie: </h4>';
	$rs2->free();

	if (!($rs3 = $db->query("SELECT name, time, rating, comment FROM Review WHERE mid = $id"))){
		$errmsg3 = $db->error;
    	print "Query failed: $errmsg3 <br />";
    	exit(1);
	}

	//table
	echo '<table border=2 cellspacing=2 cellpadding=5>';
	//header
	echo '<tr>';
	while ($column3 = $rs3->fetch_field()) { 
        echo "<th>$column3->name</th>";
	}
	echo '</tr>';

    //data
	while($row3 = $rs3->fetch_array(MYSQLI_NUM)) {
		$k = 0;
		echo '<tr>';
		while($k < $rs3->field_count){
			if($row3[$k] != null){
				echo "<td>$row3[$k]</td>";
			}
			else{
				echo '<td>N/A</td>';
			}
			$k = $k + 1;
		}
		echo '</tr>';
	}
	echo '</table>';
	echo'<br><br>';
	$rs3->free();

	if (!($avgResult = $db->query("SELECT AVG(rating) avg FROM Review WHERE mid = $id"))){
            $errmsg4 = $db->error;
            print "Query failed: $errmsg4 <br />";
            exit(1);
        } else {
          $avgArray = mysqli_fetch_assoc($avgResult);
          $avg =  $avgArray['avg'];
          if($avg==null){
          	$avg='N/A';
          }
          //echo "$did<br />";
          $avgResult->free();
          echo "<h4>Average score: $avg.<br><br></h4>";
        }

?> 

<td><a href =Add_comment.php?id=<?php echo $id ?> ><h3>Write your comment<br><br></h3></a></td>


  <?php $db->close();?>
  </body>
</html>