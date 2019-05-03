DROP TABLE IF EXISTS huanzhuang_pvp;
CREATE TABLE IF NOT EXISTS huanzhuang_pvp (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
data MEDIUMBLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS get_huanzhuang_pvp$$
CREATE PROCEDURE get_huanzhuang_pvp ()
BEGIN
SELECT data
FROM huanzhuang_pvp
WHERE gid = 1;
END$$

DROP PROCEDURE IF EXISTS save_huanzhuang_pvp$$
CREATE PROCEDURE save_huanzhuang_pvp (IN param1 MEDIUMBLOB)
BEGIN
INSERT INTO huanzhuang_pvp(gid, data)
VALUES(1, param1)
ON DUPLICATE KEY UPDATE
data = param1;
END$$

DELIMITER ;
