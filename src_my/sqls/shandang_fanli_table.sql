DROP TABLE IF EXISTS shandang_fanli;
CREATE TABLE IF NOT EXISTS shandang_fanli (
username VARCHAR(128) NOT NULL,
recharge_id VARCHAR(128) NOT NULL,
INDEX(username)
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS load_shandang_fanli$$
CREATE PROCEDURE load_shandang_fanli ()
BEGIN
SELECT username, recharge_id
FROM shandang_fanli;
END$$

DROP PROCEDURE IF EXISTS delete_shandang_fanli$$
CREATE PROCEDURE delete_shandang_fanli (IN param1 VARCHAR(128))
BEGIN
DELETE FROM shandang_fanli
WHERE username = param1;
END$$

DELIMITER ;
