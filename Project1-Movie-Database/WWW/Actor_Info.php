<html>
<body>

<?php include 'myBase.php';?>
<br><br><br><br><br><br>

<div class="col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2 main">
  <!-- <h3> <?php// echo "$_GET[id]'s";?> Movies and the role </h3> -->
  <h3> Actor/actress information(his/her movies and the roles) </h3>
   <div class="page-header">
  <h4>Actor/actress detail information: </h4>
	</div>
<?php
	$id = $_GET[id];
	if (!($rs = $db->query("SELECT id, CONCAT(first, ' ', last) fullname, sex, dob, dod FROM Actor WHERE id = $id"))){
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
	
	$rs->free();

	echo'<br><br> <div class="page-header"><h4>His/her movies and the roles: </h4></div>';

	if (!($rs2 = $db->query("SELECT M.id, M.title, MA.role FROM Movie M, MovieActor MA WHERE M.id = MA.mid AND MA.aid = $id"))){
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

		<td><a href =Movie_Info.php?id=<?php echo $row2[0]; ?> ><?php echo $row2[0]; ?></a></td>

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
	
	$rs2->free();
?> 
  <?php $db->close();?>
  </body>
</html>