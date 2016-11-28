<html>
<body>

    <?php include 'myBase.php';?>
    <br><br><br><br><br><br>

    <div class="col-sm-9 col-sm-offset-3 col-md-5 col-md-offset-2 main">
        <h3>Add a movie comment</h3>
        <form action="Add_comment.php" method="POST">

            <div class="form-group">
                <label for="sel1">Movie title</label>
                <select class="form-control" name="title">
                    <?php if($_GET[id]){
                        if (!($selectMovieTitle = $db->query("SELECT title FROM Movie WHERE id = $_GET[id]"))){
                            $errmsg2 = $db->error;
                            print "Query failed: $errmsg2 <br />";
                            exit(1);
                        } else {
                            $selectMovieTitleArray = mysqli_fetch_assoc($selectMovieTitle);
                            $sTitle = $selectMovieTitleArray['title'];
                            $selectMovieTitle -> free();?>
                            <option selected><?php echo $sTitle?></option>
                            <?php
                        } 

                    }
                    ?>
                    <?php
                    if(!($result = $db->query("SELECT title FROM Movie ORDER BY title ASC"))){
                        $errmsg = $db->error;
                        print "Query failed: $errmsg <br/>";
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
                        <label for="your_name">Your Name</label>
                        <input type="text" class="form-control" placeholder="Text input" name="name"/>
                    </div>

                    <div class="form-group">
                        <label for="your_rating">Your rating for the movie </label>
                        <input type="text" class="form-control" placeholder="very good-5, excellent-4, good-3, not good-2, bad-1, terrible-0" name="rating"/>
                    </div>

<!--         <div class="form-group">
        <label for="sel2">Your rating for the movie (0~5)</label>
        <select class="form-control" name="rating">
          <option>very good - 5</option>
          <option>excellent - 4</option>
          <option>good      - 3</option>
          <option>not good  - 2</option>
          <option>bad       - 1</option>
          <option>terrible  - 0</option>
        </select>
        </div>
    -->
    <div class="form-group">
        <label for="comment">Your comment for the movie (no more than 500 characters)</label>
        <textarea class="form-control" rows="5" placeholder="Text input" name="comment"></textarea>
    </div>


    <button type="submit" class="btn btn-default">Submit!</button>
</form>
</div>

<?php
 //todo:增加处理链接来的信息，绑定相关值！
if ($_POST[title] && $_POST[name] && $_POST[rating] && $_POST[comment]){
    if ($_POST[rating] >= 0 && $_POST[rating] <= 5) {
      //echo 'valid!!!!!';
    // $ratingInput = $_POST[rating];
    // switch($ratingInput){
    //     case "very good -- 5":
    //         $rating = 5;
    //         break;
    //     case "excellent -- 4":
    //         $rating = 4;
    //         break;
    //     case "good      -- 3":
    //         $rating = 3;
    //         break;
    //     case "not good  -- 2":
    //         $rating = 2;
    //         break;
    //     case "bad       -- 1":
    //         $rating = 1;
    //         break;
    //     case "terrible  -- 0":
    //         $rating = 0;
    //         break;
    //     default:
    //         $rating = 5;
    // }
      if (!($midResult =$db -> query("SELECT id From Movie WHERE title = '$_POST[title]' "))){
        $errmsg = $db -> error;
        print "Query failed: $errmsg <br/>";
        exit(1);
    } else{
        $midArray = mysqli_fetch_assoc($midResult);
        $mid = $midArray['id'];
        //echo "$mid<br/>";
        $midResult -> free();
    }

    $insert = "INSERT INTO Review(name, time, mid, rating, comment) VALUES('$_POST[name]', NOW(), $mid, $_POST[rating],
    '$_POST[comment]')";
      // $insert = "INSERT INTO Review(name, time, mid, rating, comment) VALUES('$_POST[name]', NOW(), $mid, $rating,
      // '$_POST[comment]')";
    if (!$db -> query($insert)) {
        $errmsg = $db -> error;
        print "Query failed: $errmsg <br>";
        exit(1);
    } else {
        echo '<h4>Submit successfully! Thank u~<br></h4>';
    }
} else {
  echo "<h4> Invaild input! Rating should be 0~5.</h4>";
}
} else{
    if ($_POST[title] || $_POST[name] || $_POST[rating] || $_POST[comment]){
      echo '<h4>Failed! Some information is empty. <br>Please complete all the information! </h4>';
  }
}
?>

<?php $db->close();?>
</body>
</html>