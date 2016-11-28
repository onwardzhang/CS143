<html>
<body>

<?php include 'myBase.php';?>
<br><br><br><br><br><br>

<div class="col-sm-9 col-sm-offset-3 col-md-5 col-md-offset-2 main">
  <h3>Search for an actor/actress/movie </h3>
  <form action="search.php" method="POST">
  <div class="form-group">
          <input type="text" class="form-control" placeholder="Keyword input"  name="search"/>
  </div>
  <button type="submit" class="btn btn-default">Search!</button>
    </form>

<?php
  if($_POST[search]){
  	?>

  <div class="page-header">
  <h4>Matching actor/actress: </h4>
</div>
  	<?php
  	$search = $_POST[search];
	$searchArray = explode(" ", $search);
	//echo $search;
	$len = count($searchArray);
	//echo $len;

	if($len == 2 || $len == 1 ){
		if ($len == 1 ){
			$query1 = "SELECT id, CONCAT (first, ' ', last) fullname, sex, dob FROM Actor WHERE CONCAT (first, ' ', last) LIKE '%$searchArray[0]%' ORDER BY fullname";
		} 
		else {
			$query1 = "SELECT id, CONCAT (first, ' ', last) fullname, sex, dob FROM Actor WHERE CONCAT (first, ' ', last) LIKE '%$searchArray[0]%' AND CONCAT (first, ' ', last) LIKE '%$searchArray[1]%' ORDER BY fullname";
		}
		//echo $query1;

	if (!($rs = $db->query($query1))){
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
	//echo'<br><br>';
	$rs->free();
	}

	$query2 = "SELECT id, title, year FROM Movie WHERE title LIKE '%$searchArray[0]%'";
	for ($j = 1; $j < $len; $j++) {
		$query2 .= " AND title LIKE '%$searchArray[$j]%'";
	}
	$query2 .= " ORDER BY title";
	//echo $query2;

	if (!($rs2 = $db->query($query2))){
		$errmsg2 = $db->error;
    	print "Query failed: $errmsg2 <br />";
    	exit(1);
		}
	?>
  <div class="page-header">
  <h4>Matching movies: </h4>
</div>
	<?php
	//table
	echo '<table border=2 cellspacing=5 cellpadding=5>';
	//header
	echo '<tr>';
	while ($column2 = $rs2->fetch_field()) { 
        echo "<th>$column2->name</th>";
	}
	echo '</tr>';

    //data
	while($row2 = $rs2->fetch_array(MYSQLI_NUM)) {
		$k = 1;
		echo '<tr>';
		?>

		<td><a href =Movie_Info.php?id=<?php echo $row2[0]; ?> ><?php echo $row2[0]; ?></a></td>

		<?php
		while($k < $rs2->field_count){
			if($row2[$k] != null){
				echo "<td>$row2[$k]</td>";
			}
			else{
				echo '<td>Unknown</td>';
			}
			$k++;
		}
		echo '</tr>';
	}
	echo '</table>';
	echo '<br><br>';
	//echo'<br><br>';
	$rs2->free();

}
?>

<?php $db->close();?>
</div>
</body>
</html>