CREATE TABLE Movie(
	id INT NOT NULL,
	title VARCHAR(100) NOT NULL,
	year INT NOT NULL,
	rating VARCHAR(10) NOT NULL,
	company VARCHAR(50) NOT NULL,
	PRIMARY KEY(id)) ENGINE=INNODB;
	#id is unique to each movie, so I choose id as the primary key

CREATE TABLE Actor(
	id INT NOT NULL,
	last VARCHAR(20) NOT NULL,
	first VARCHAR(20) NOT NULL,
	sex VARCHAR(6),
	dob DATE NOT NULL,
	dod DATE,
	PRIMARY KEY(id),
	CHECK (dod<=>NULL OR dob<dod)) ENGINE=INNODB;
	#id is unique to each actor, so I choose id as the primary key
	#a person can only die after he/she was born(dob<dod), 
		#or he/she is still alive(dod<=>NULL,NULL=NULL returns NULL,NULL<=>NULL returns 1).

CREATE TABLE Director(
	id INT NOT NULL,
	last VARCHAR(20) NOT NULL,
	first VARCHAR(20) NOT NULL,
	dob DATE NOT NULL,
	dod DATE,
	PRIMARY KEY(id),
	CHECK (dod<=>NULL OR dob<dod)) ENGINE=INNODB;
	#id is unique to each director, so I choose id as the primary key
	#a person can only die after he/she was born(dob<dod), 
		#or he/she still is alive(dod<=>NULL,NULL=NULL returns NULL,NULL<=>NULL returns 1).

CREATE TABLE MovieGenre(
	mid INT NOT NULL,
	genre VARCHAR(20) NOT NULL,
	FOREIGN KEY(mid) REFERENCES Movie(id)) ENGINE=INNODB;
#mid in MovieGenre must exist in the table of Movie

CREATE TABLE MovieDirector(
	mid INT NOT NULL, 
	did INT NOT NULL,
	FOREIGN KEY(mid) REFERENCES Movie(id),
	FOREIGN KEY(did) REFERENCES Director(id)) ENGINE=INNODB;
#mid in MovieDirector must exist in attribute(id) of Movie
#did in MovieDirector must exist in attribute(id) of Director

CREATE TABLE MovieActor(
	mid INT NOT NULL,
	aid INT NOT NULL,
	role VARCHAR(50),
	FOREIGN KEY(mid) REFERENCES Movie(id),
	FOREIGN KEY(aid) REFERENCES Actor(id)) ENGINE=INNODB;
#mid in MovieActor must exist in attribute(id) of Movie
#aid in MovieActor must exist in attribute(id) of Actor

CREATE TABLE Review(
	name VARCHAR(20) NOT NULL,
	time TIMESTAMP NOT NULL,
	mid INT NOT NULL,
	rating INT NOT NULL,
	comment VARCHAR(500),
	FOREIGN KEY(mid) REFERENCES Movie(id),
	CHECK (0<=rating AND rating<=5)) ENGINE=INNODB;
#mid in Review must exist in attribute(id) of Movie
#according to the tutorial, rating is x out of 5, therefore 0<=rating<=5

CREATE TABLE MaxPersonID(id INT);
CREATE TABLE MaxMovieID(id INT);
