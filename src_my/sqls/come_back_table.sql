DROP TABLE IF EXISTS come_back;
CREATE TABLE IF NOT EXISTS come_back (
username VARCHAR(128) NOT NULL UNIQUE
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS load_come_back$$
CREATE PROCEDURE load_come_back ()
BEGIN
SELECT username
FROM come_back;
END$$

DROP PROCEDURE IF EXISTS delete_come_back$$
CREATE PROCEDURE delete_come_back (IN param1 VARCHAR(128))
BEGIN
DELETE FROM come_back
WHERE username = param1;
END$$

DELIMITER ;
