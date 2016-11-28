<html>
<body>

<?php include 'myBase.php';?>
<br><br><br><br><br><br>

<div class="col-sm-9 col-sm-offset-3 col-md-5 col-md-offset-2 main">
  <h3>Add a new movie</h3>
  <form action="Add_movie.php" method="POST">

        <div class="form-group">
          <label for="movieTitle">Title</label>
          <input type="text" class="form-control" placeholder="Text input"  name="title"/>
        </div>

        <div class="form-group">
          <label for="movieYear">Year</label>
          <input type="text" class="form-control" placeholder="Text input" name="year"/>
        </div>

        <div class="form-group">
        <label for="sel1">MPAA rating</label>
        <select class="form-control" name="rating">
          <option>G</option>
          <option>PG</option>
          <option>PG-13</option>
          <option>R</option>
          <option>NC-17</option>
        </select>
        </div>

        <div class="form-group">
          <label for="movieCompany">Company</label>
          <input type="text" class="form-control" placeholder="Text input" name="company"/> 
        </div>
 
    <label for="movieGenre">Genre</label>
    <br>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="action">action</label> <!-- genre[],[]必须加 -->
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="animated">animated</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="adventure">adventure</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="comedy">comedy</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="crime">crime</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="drama">drama</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="disaster">disaster</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="epic">epic</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="fantasy">fantasy</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="horror">horror</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="musicals">musicals</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="sports">sports</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="superhero">superhero</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="war">war</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="westerns">westerns</label>
    <label class="checkbox-inline"><input type="checkbox" name='genre[]' value="others">others</label>
    <br><br>
    <button type="submit" class="btn btn-default">Add!</button>
    </form>
</div>

 <?php
    if ($_POST[title] && $_POST[year] && $_POST[company] && $_POST[rating] && $_POST[genre]){

        if (!($movieIDResult = $db->query("SELECT id From MaxMovieID"))){
            $errmsg = $db->error;
            print "Query failed: $errmsg <br />";
            exit(1);
        } else {
          $movieID = mysqli_fetch_assoc($movieIDResult);
          $newMaxMovieID =  $movieID['id']+1;
          //echo "$newMaxMovieID<br />";
          $movieIDResult->free();
        }
        $query = "INSERT INTO Movie(id,title,year,rating,company) VALUES ($newMaxMovieID,'$_POST[title]','$_POST[year]','$_POST[rating]', '$_POST[company]')";

    if (!$db->query($query)){
      $errmsg = $db->error;
      print "Query failed: $errmsg <br />";
      exit(1);
    } else{
      //echo 'Add into successfully!';

      $update = "UPDATE MaxMovieID SET id=$newMaxMovieID";
      if (!$db->query($update)){
        $errmsg = $db->error;
        print "Query failed: $errmsg <br>";
        exit(1);
      } else{
        echo '<h4>Update MaxMovieID successfully!<br></h4>';
      }
    }

    for($i=0; $i < count($_POST[genre]); $i++){
        $newGenre = $_POST[genre][$i];
        $insertToMG = "INSERT INTO MovieGenre(mid ,genre) VALUES ($newMaxMovieID, '$newGenre')";
        //echo "$insert<br>";
        if (!$db->query($insertToMG)){
          $errmsg = $db->error;
          print "Query failed: $errmsg <br />";
          exit(1);
      }
    }
    echo '<h4>insertToMG successfully!<br></h4>';
  } else {
      if ($_POST[title] || $_POST[year] || $_POST[company] || $_POST[rating] || $_POST[genre]){
         echo '<h4>Failed! Some information is empty. <br>Please complete all the information! </h4>';
      }
    }
?>

<?php $db->close();?>
</body>
</html>