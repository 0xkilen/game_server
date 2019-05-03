DROP TABLE IF EXISTS used_rolenames;
CREATE TABLE IF NOT EXISTS used_rolenames (
gid BIGINT UNSIGNED UNIQUE,
data BLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS load_used_rolenames$$
CREATE PROCEDURE load_used_rolenames ()
BEGIN
SELECT data
FROM used_rolenames
WHERE gid = 1;
END$$

DROP PROCEDURE IF EXISTS update_used_rolenames$$
CREATE PROCEDURE update_used_rolenames (IN param1 BLOB)
BEGIN
INSERT INTO used_rolenames(gid, data)
VALUES(1, param1)
ON DUPLICATE KEY UPDATE
data = param1;
END$$

DELIMITER ;
