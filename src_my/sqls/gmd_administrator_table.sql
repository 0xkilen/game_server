DROP TABLE IF EXISTS gmd_administrators;
CREATE TABLE IF NOT EXISTS gmd_administrators (
        gid BIGINT PRIMARY KEY AUTO_INCREMENT,
        username VARCHAR(128) UNIQUE NOT NULL,
        password VARCHAR(128) UNIQUE NOT NULL
) AUTO_INCREMENT = 1
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS load_gmd_administrators$$
CREATE PROCEDURE load_gmd_administrators ()
BEGIN
SELECT username, password
FROM gmd_administrators;
END$$

DROP PROCEDURE IF EXISTS add_gmd_administrator$$
CREATE PROCEDURE add_gmd_administrator (IN param1 VARCHAR(128), IN param2 VARCHAR(128))
BEGIN
INSERT INTO gmd_administrators(username, password)
VALUES(param1, param2);
END$$

DROP PROCEDURE IF EXISTS update_gmd_administrator$$
CREATE PROCEDURE update_gmd_administrator (IN param1 VARCHAR(128), IN param2 VARCHAR(128))
BEGIN
UPDATE gmd_administrators
SET password = param2
WHERE username = param1;
END$$

DROP PROCEDURE IF EXISTS delete_gmd_administrator$$
CREATE PROCEDURE delete_gmd_administrator (IN param1 VARCHAR(128))
BEGIN
DELETE FROM gmd_administrators
WHERE username = param1;
END$$

DELIMITER ;

set @username = 'admin';
set @password = 'jxwy';
call add_gmd_administrator(@username, @password);
