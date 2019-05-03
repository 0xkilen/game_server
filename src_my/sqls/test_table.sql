CREATE TABLE IF NOT EXISTS users (
user_name VARCHAR(24)
);

DELIMITER $$

DROP PROCEDURE IF EXISTS add_user$$
CREATE PROCEDURE add_user (IN param1 VARCHAR(24))
BEGIN
	INSERT INTO users(user_name)
	VALUES(param1);
END$$

DROP PROCEDURE IF EXISTS find_user$$
CREATE PROCEDURE find_user (IN param1 VARCHAR(24))
BEGIN
	SELECT *
	FROM users
	WHERE user_name = param1;
END$$

DROP PROCEDURE IF EXISTS delete_user$$
CREATE PROCEDURE delete_user (IN param1 VARCHAR(24))
BEGIN
	DELETE FROM users
	WHERE user_name = param1;
END$$

DELIMITER ;
