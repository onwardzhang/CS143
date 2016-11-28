<html>
<body>

<?php include 'myBase.php';?>
<br><br><br><br><br><br>

<div class="col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-1 main">
  <h3>Browsing Actor/Actress Information</h3>
  <h5> Click on the id to see the movies that the actor/actress was in.</h5>

  <?php
	if (!($rs = $db->query("SELECT id, CONCAT(first, ' ', last) fullname, sex, dob, dod FROM Actor"))){
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
		$i=1;
		echo '<tr>';
		?>

		<td><a href =Actor_Info.php?id=<?php echo $row[0]; ?> ><?php echo $row[0]; ?></a></td>

		<?php
		while($i<$rs->field_count){
			if($row[$i]!=null){
				echo "<td>$row[$i]</td>";
			}
			else{
				echo '<td>Unknown</td>';
			}
			$i=$i+1;
		}
		echo '</tr>';
	}
	echo '</table>';
	
	$rs->free();
?> 

<?php $db->close();?>
  </body>
</html>