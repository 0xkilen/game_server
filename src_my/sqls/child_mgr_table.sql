DROP TABLE IF EXISTS child_mgr;
CREATE TABLE IF NOT EXISTS child_mgr (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
data MEDIUMBLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS get_child_mgr$$
CREATE PROCEDURE get_child_mgr ()
BEGIN
SELECT data
FROM child_mgr
WHERE gid = 1;
END$$

DROP PROCEDURE IF EXISTS save_child_mgr$$
CREATE PROCEDURE save_child_mgr (IN param1 MEDIUMBLOB)
BEGIN
INSERT INTO child_mgr(gid, data)
VALUES(1, param1)
ON DUPLICATE KEY UPDATE
data = param1;
END$$

DELIMITER ;
