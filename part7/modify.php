<html>

 <head><title>CS 564 PHP Project Modify Result Page</title></head>

 <body>      
	
 <?php
   // First check the itemid to see if it has been set
  if (!isset($_POST['title']) ) {
    echo "  <h3><i>Error, title not set to an acceptable value</i></h3>\n".
        " <a href=\"https://cs564.cs.wisc.edu/halit/index.html\">Back to main page</a>\n".
	" </body>\n</html>\n";
    exit();
  }
  $title = $_POST['title'];
  $author = $_POST['author'];
  $num = $_POST['num'];
  // Connect to the Database
  pg_connect('dbname=cs564_f12 host=postgres.cs.wisc.edu') 
	or die ("Couldn't Connect ".pg_last_error()); 
  // Get category name and item counts
  if( strlen($author) > 0 || strlen($num) > 0 )	{
	$query = "update stage7_book_schema.book set";
	if( strlen($author) )	{
	$query .= " author='".$author."'";
	}
	if( strlen($author) && strlen($num) )	{
	$query .= ",";
	}
	if( strlen($num) )	{
	$query .= " num=".$num;
	}
	$query .= " where title='".$title."'";
  }else	{
    echo "  <h3><i>no field to be updated</i></h3>\n".
        " <a href=\"https://cs564.cs.wisc.edu/halit/index.html\">Back to main page</a>\n".
	" </body>\n</html>\n";
    exit();
  }
  // Execute the query and check for errors
  $result = pg_query($query);
  if (!$result) {
    $errormessage = pg_last_error();
    echo "Error with query: " . $errormessage;
    exit();
  }
  echo "  <h3>Update Successful</h3>";
  
  pg_close();
?>

        <?php echo "<a href=\"https://cs564.cs.wisc.edu/halit/index.html\">Back to main page</a>\n"?>
 </body>

</html>
