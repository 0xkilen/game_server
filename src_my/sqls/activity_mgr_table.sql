DROP TABLE IF EXISTS activity_mgr;
CREATE TABLE IF NOT EXISTS activity_mgr (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
data MEDIUMBLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS get_activity_mgr$$
CREATE PROCEDURE get_activity_mgr ()
BEGIN
SELECT data
FROM activity_mgr
WHERE gid = 1;
END$$

DROP PROCEDURE IF EXISTS save_activity_mgr$$
CREATE PROCEDURE save_activity_mgr (IN param1 MEDIUMBLOB)
BEGIN
INSERT INTO activity_mgr(gid, data)
VALUES(1, param1)
ON DUPLICATE KEY UPDATE
data = param1;
END$$

DELIMITER ;
