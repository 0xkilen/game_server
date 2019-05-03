DROP TABLE IF EXISTS logs;
CREATE TABLE IF NOT EXISTS logs (
rolegid BIGINT UNSIGNED NOT NULL,
time INT UNSIGNED NOT NULL,
type INT UNSIGNED NOT NULL,
data BLOB NOT NULL
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS add_log$$
CREATE PROCEDURE add_log (IN param1 BIGINT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 BLOB)
BEGIN
INSERT INTO logs(rolegid, time, type, data)
VALUES(param1, param2, param3, param4);
END$$

DROP PROCEDURE IF EXISTS get_log$$
CREATE PROCEDURE get_log (IN param1 INT, IN param2 INT)
BEGIN
SELECT rolegid, time, type, data
FROM logs
LIMIT param1, param2;
END$$

DROP PROCEDURE IF EXISTS find_log$$
CREATE PROCEDURE find_log (IN param1 BIGINT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT, IN param5 INT)
BEGIN
SELECT time, type, data
FROM logs
WHERE rolegid = param1 AND time >= param2 AND time <= param3
ORDER BY time DESC
LIMIT param4, param5;
END$$

DROP PROCEDURE IF EXISTS find_mail_log_by_mail_gid$$
CREATE PROCEDURE find_mail_log_by_mail_gid (IN param1 BIGINT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT UNSIGNED, IN param5 INT UNSIGNED, IN param6 INT UNSIGNED, IN param7 INT UNSIGNED, IN param8 INT UNSIGNED, IN param9 INT)
BEGIN
SELECT rolegid, type, data
FROM logs
WHERE rolegid = param1 AND (type = param2 OR type = param3 OR type = param4 OR type = param5) AND time >= param6 AND time <= param7
ORDER BY time DESC
LIMIT param8, param9;
END$$

DROP PROCEDURE IF EXISTS find_mail_log$$
CREATE PROCEDURE find_mail_log (IN param1 INT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT UNSIGNED, IN param5 INT UNSIGNED, IN param6 INT UNSIGNED, IN param7 INT UNSIGNED, IN param8 INT)
BEGIN
SELECT rolegid, type, data
FROM logs
WHERE (type = param1 OR type = param2 OR type = param3 OR type = param4) AND time >= param5 AND time <= param6
ORDER BY time DESC
LIMIT param7, param8;
END$$

DROP PROCEDURE IF EXISTS find_logs_by_gid_and_one_type$$
CREATE PROCEDURE find_logs_by_gid_and_one_type (IN param1 BIGINT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT UNSIGNED, IN param5 INT, IN param6 INT)
BEGIN
SELECT time, type, data
FROM logs
WHERE rolegid = param1 AND type = param2 AND time >= param3 AND time <= param4
ORDER BY time DESC
LIMIT param5, param6;
END$$

DROP PROCEDURE IF EXISTS find_logs_by_gid_and_three_type$$
CREATE PROCEDURE find_logs_by_gid_and_three_type (IN param1 BIGINT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT UNSIGNED, IN param5 INT UNSIGNED, IN param6 INT UNSIGNED, IN param7 INT, IN param8 INT)
BEGIN
SELECT time, type, data
FROM logs
WHERE rolegid = param1 AND (type = param2 OR type = param3 OR type = param4) AND time >= param5 AND time <= param6
ORDER BY time DESC
LIMIT param7, param8;
END$$

DROP PROCEDURE IF EXISTS find_logs_type$$
CREATE PROCEDURE find_logs_by_type (IN param1 INT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT, IN param5 INT)
BEGIN
SELECT time, type, data
FROM logs
WHERE type = param1 AND time >= param2 AND time <= param3
ORDER BY time DESC
LIMIT param4, param5;
END$$

DROP PROCEDURE IF EXISTS find_logs_by_two_type$$
CREATE PROCEDURE find_logs_by_two_type (IN param1 INT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT UNSIGNED, IN param5 INT, IN param6 INT)
BEGIN
SELECT rolegid, time, data
FROM logs
WHERE (type = param1 OR type = param2) AND time >= param3 AND time <= param4
ORDER BY time DESC
LIMIT param5, param6;
END$$

DROP PROCEDURE IF EXISTS find_logs_by_gid_and_two_type$$
CREATE PROCEDURE find_logs_by_gid_and_two_type (IN param1 BIGINT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT UNSIGNED, IN param5 INT UNSIGNED, IN param6 INT, IN param7 INT)
BEGIN
SELECT time, type, data
FROM logs
WHERE rolegid = param1 AND (type = param2 OR type = param3) AND time >= param4 AND time <= param5
ORDER BY time DESC
LIMIT param6, param7;
END$$

DROP PROCEDURE IF EXISTS find_logs_by_five_type$$
CREATE PROCEDURE find_logs_by_five_type (IN param1 INT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT UNSIGNED, IN param5 INT UNSIGNED, IN param6 INT UNSIGNED, IN param7 INT UNSIGNED, IN param8 INT, IN param9 INT)
BEGIN
SELECT rolegid, time, data
FROM logs
WHERE (type = param1 OR type = param2 OR type = param3 OR type = param4 OR type = param5) AND time >= param6 AND time <= param7
ORDER BY time ASC
LIMIT param8, param9;
END$$

DROP PROCEDURE IF EXISTS find_logs_by_gid_and_five_type$$
CREATE PROCEDURE find_logs_by_gid_and_five_type (IN param1 BIGINT UNSIGNED, IN param2 INT UNSIGNED, IN param3 INT UNSIGNED, IN param4 INT UNSIGNED, IN param5 INT UNSIGNED, IN param6 INT UNSIGNED, IN param7 INT UNSIGNED, IN param8 INT UNSIGNED, IN param9 INT, IN param10 INT)
BEGIN
SELECT rolegid, time, data
FROM logs
WHERE rolegid = param1 AND (type = param2 OR type = param3 OR type = param4 OR type = param5 OR type = param6) AND time >= param7 AND time <= param8
ORDER BY time ASC
LIMIT param9, param10;
END$$

DELIMITER ;
