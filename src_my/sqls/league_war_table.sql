DROP TABLE IF EXISTS league_war;
CREATE TABLE IF NOT EXISTS league_war (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
data MEDIUMBLOB NOT NULL
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$
DROP PROCEDURE IF EXISTS get_league_war$$
CREATE PROCEDURE get_league_war ()
BEGIN
SELECT data
FROM league_war
WHERE gid = 1;
END$$

DROP PROCEDURE IF EXISTS save_league_war$$
CREATE PROCEDURE save_league_war (IN param1 MEDIUMBLOB)
BEGIN
INSERT INTO league_war(gid , data)
VALUES(1, param1)
ON DUPLICATE KEY UPDATE
data = param1;
END$$

DELIMITER ;
