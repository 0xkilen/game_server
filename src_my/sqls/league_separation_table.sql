DROP TABLE IF EXISTS league_separations;
CREATE TABLE IF NOT EXISTS league_separations (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
data MEDIUMBLOB NOT NULL
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS add_league_separation$$
CREATE PROCEDURE add_league_separation (IN param1 BIGINT UNSIGNED, IN param2 MEDIUMBLOB)
BEGIN
INSERT INTO league_separations(gid, data)
VALUES(param1, param2);
END$$

DROP PROCEDURE IF EXISTS find_league_separation$$
CREATE PROCEDURE find_league_separation (IN param1 BIGINT UNSIGNED)
BEGIN
SELECT data
FROM league_separations
WHERE gid = param1;
END$$

DROP PROCEDURE IF EXISTS get_league_separations$$
CREATE PROCEDURE get_league_separations ()
BEGIN
SELECT gid, data
FROM league_separations;
END$$

DROP PROCEDURE IF EXISTS delete_league_separation$$
CREATE PROCEDURE delete_league_separation (IN param1 BIGINT UNSIGNED)
BEGIN
DELETE FROM league_separations
WHERE gid = param1;
END$$

DROP PROCEDURE IF EXISTS update_league_separation$$
CREATE PROCEDURE update_league_separation (IN param1 BIGINT UNSIGNED, IN param2 MEDIUMBLOB)
BEGIN
UPDATE league_separations
SET data = param2
WHERE gid = param1;
END$$

DELIMITER ;
