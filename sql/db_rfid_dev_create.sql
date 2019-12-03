CREATE DATABASE IF NOT EXISTS rfid_dev DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

use rfid_dev;

CREATE TABLE IF NOT EXISTS `t_operator` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pwd` char(40) NOT NULL,
  `name` varchar(32) NOT NULL,
  `phone` varchar(12) NOT NULL,
  `gender` tinyint NOT NULL,  -- 0:male 1:female
  `status` tinyint NOT NULL DEFAULT 0, -- 0:normal 1:frozen 2:deleted
  `role` int NOT NULL, -- 1:admin 2:HeadNurse 3:CssdUser 4:OrUser 5:ClinicUser
  `dept_id` int NOT NULL,
  -- `is_admin` tinyint NOT NULL DEFAULT 0,
  `is_online` tinyint NOT NULL DEFAULT 0, -- 0:offline 1:online
  `last_login_time` datetime
) ENGINE=InnoDB AUTO_INCREMENT=110001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_dept` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `pinyin` varchar(32) NOT NULL,
  `is_surgical` tinyint NOT NULL DEFAULT 0,
  `phone` varchar(12),
  `location` varchar(64),
  `is_del` tinyint NOT NULL DEFAULT 0
) ENGINE=InnoDB AUTO_INCREMENT=120001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_device` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `category` tinyint NOT NULL, -- 0:wash 1:sterilize
  `status` tinyint NOT NULL DEFAULT 0, -- 0:idle 1:running 2:forbidden
  `cycle_count` smallint NOT NULL DEFAULT 0, -- cycle count for the most recent running
  `cycle_date` datetime NOT NULL, -- the last time at when it started
  `cycle_total` int NOT NULL,
  `sterilize_type` tinyint NOT NULL DEFAULT 0, -- 0:both 1:low 2:high -1:N/A
  `production_time` datetime NOT NULL,
  `last_maintain_time` timestamp DEFAULT CURRENT_TIMESTAMP,
  `maintain_cycle` smallint NOT NULL DEFAULT 0, -- in days
  `is_del` tinyint NOT NULL DEFAULT 0
) ENGINE=InnoDB AUTO_INCREMENT=130001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_program` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `category` tinyint NOT NULL, -- 0:wash 1:sterilize
  `name` varchar(32) NOT NULL,
  `remark` varchar(128),
  `is_del` tinyint NOT NULL DEFAULT 0
) ENGINE=InnoDB AUTO_INCREMENT=140001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_device_programs` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `device_id` int NOT NULL,
  `program_id` int NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=150001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_pack_type` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `name` varchar(16) NOT NULL,
  `valid_period` smallint unsigned NOT NULL,
  `standard_period` smallint unsigned NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=160001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_package_type` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `category` tinyint NOT NULL DEFAULT 0, -- For backward compatibility, 0:surgical 1:clinical 2:external 3:dressing 4:universal
  `name` varchar(32) NOT NULL,
  `pinyin` varchar(32) NOT NULL,
  `photo` varchar(128),
  `sterilize_type` tinyint NOT NULL DEFAULT 0, -- 0:both 1:low 2:high
  `pack_type_id` int NOT NULL,
  `dept_id` int NOT NULL,
  `is_del` tinyint NOT NULL DEFAULT 0
  -- `num` smallint NOT NULL DEFAULT 0
) ENGINE=InnoDB AUTO_INCREMENT=220001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_package` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `udi` varchar(32) NOT NULL UNIQUE,
  `type_id` int NOT NULL,
  `name` varchar(32) NOT NULL,
  `photo` varchar(128),
  `cycle` int unsigned NOT NULL DEFAULT 0, -- +1 after wash
  -- 0:unknown 1:washed 2:wash-checked 3:packed 4:sterilized 5:ster-checked-pass 6:ster-checked-failed
  -- 7:dispatched 8:received 9:pre-op-checked 10:post-op-checked 11:recycled 99:recalled
  `status` tinyint(2) NOT NULL DEFAULT 0,
  `is_del` tinyint NOT NULL DEFAULT 0,
  KEY `idx_udi` (`udi`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_instrument_type` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `category` tinyint NOT NULL DEFAULT 0, -- Reserved for future, 0:normal 1:implanted
  `name` varchar(32) NOT NULL,
  `pinyin` varchar(32) NOT NULL,
  `photo` varchar(128),
  `is_vip` tinyint NOT NULL DEFAULT 0,
  `is_del` tinyint NOT NULL DEFAULT 0
) ENGINE=InnoDB AUTO_INCREMENT=240001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_instrument` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `udi` varchar(32) NOT NULL UNIQUE,
  `type_id` int NOT NULL,
  `name` varchar(32) NOT NULL,
  `photo` varchar(128),
  `price` int,  -- in cents
  `pkg_udi` varchar(32), -- always points to the most recent bound package
  `cycle` int unsigned NOT NULL DEFAULT 0, -- update after wash
  `is_del` tinyint NOT NULL DEFAULT 0,
  KEY `idx_udi` (`udi`) USING BTREE,
  KEY `idx_pkg_udi` (`pkg_udi`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_package_type_detail` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pkg_type_id` int NOT NULL,
  `ins_type_id` int NOT NULL,
  `num` smallint NOT NULL,
  KEY `idx_pkg_type_id` (`pkg_type_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_package_detail` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle_stamp` int unsigned NOT NULL,
  `ins_udi` varchar(32) NOT NULL,
  `ins_cycle_stamp` int unsigned NOT NULL,
  `status` tinyint(1) NOT NULL default 1, -- 0:unbound 1:bound
  `bound_tm` datetime NOT NULL,
  `unbound_tm` datetime,
  KEY `idx_ins_udi` (`ins_udi`) USING BTREE,
  KEY `idx_pkg_udi` (`pkg_udi`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Below are flow tables, note that records for flow are snapshots.
-- For example, a udi-package may be bound to another package type or department,
-- thus we have to log the package type/name/dept at that time

-- `r_package` recoreds a snapshot of the static package properties, as well as the recall information. Insert after a packge is washed
CREATE TABLE IF NOT EXISTS `r_package` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle` int unsigned NOT NULL,
  `pkg_name` varchar(32) NOT NULL,
  `pkg_type_id` int NOT NULL,
  `pkg_type_name` varchar(32) NOT NULL,
  `dept_id` int NOT NULL, -- dept it belongs to
  `dept_name` varchar(32) NOT NULL,
  -- 1:washed 2:wash-checked 3:packed 4:sterilized 5:ster-checked-pass 6:ster-checked-failed
  -- 7:dispatched 8:received 9:pre-op-checked 10:post-op-checked 11:recycled
  `status` tinyint(2) NOT NULL DEFAULT 1, -- stop updating after recalled
  `recalled` tinyint(1) NOT NULL DEFAULT 0, -- 0:no 1:yes
  KEY `idx_pkg_udi_cycle` (`pkg_udi`,`pkg_cycle`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `r_recycle` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle` int unsigned NOT NULL,
  `pkg_name` varchar(32) NOT NULL,
  `from_dept_id` int DEFAULT 0, -- maybe null
  `from_dept_name` varchar(32) DEFAULT NULL,
  `op_id` int NOT NULL,
  `op_name` varchar(32) NOT NULL,
  `recycle_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  KEY `idx_pkg_udi_cycle` (`pkg_udi`,`pkg_cycle`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- This table records wash batches rather than packages
CREATE TABLE IF NOT EXISTS `r_wash_batch` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `batch_id` varchar(32) NOT NULL,
  `device_id` int NOT NULL,
  `device_name` varchar(32) NOT NULL,
  `program_id` int NOT NULL,
  `program_name` varchar(32) NOT NULL,
  `cycle_count` int(11) NOT NULL,
  `cycle_total` int(11) NOT NULL,
  `start_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `finish_time` datetime,
  `op_id` int NOT NULL,
  `op_name` varchar(32) NOT NULL,
  `check_op_id` int DEFAULT '0',
  `check_op_name` varchar(32),
  `check_result` tinyint NOT NULL DEFAULT 1, -- 0:unqualified 1:qualified 2:partial-qualified
  `check_desc` varchar(256),
  `check_time` datetime,
  KEY `idx_batch_id` (`batch_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `r_wash_package` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle` int unsigned NOT NULL,
  `pkg_name` varchar(32) NOT NULL,
  `batch_id` varchar(32) NOT NULL,
  KEY `idx_pkg_udi_cycle` (`pkg_udi`,`pkg_cycle`) USING BTREE,
  KEY `idx_batch_id` (`batch_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `r_pack` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT, -- label_id, maybe inappropriate
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle` int unsigned NOT NULL,
  `pkg_name` varchar(32) NOT NULL,
  `dept_id` int NOT NULL,
  `dept_name` varchar(32) NOT NULL,
  `op_id` int NOT NULL,
  `op_name` varchar(32) NOT NULL,
  `pack_type_id` int NOT NULL,
  `pack_type_name` varchar(32) NOT NULL,
  `pack_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `expire_time` timestamp NOT NULL,
  `check_op_id` int DEFAULT 0,
  `check_op_name` varchar(32) DEFAULT NULL,
  `check_time` datetime,
  -- `check_result` tinyint(1) DEFAULT 1,
  KEY `idx_pkg_udi_cycle` (`pkg_udi`,`pkg_cycle`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=310001 DEFAULT CHARSET=utf8;

-- This table records sterilize batches rather than packages
CREATE TABLE IF NOT EXISTS `r_ster_batch` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `batch_id` varchar(32) NOT NULL,
  `device_id` int NOT NULL,
  `device_name` varchar(32) NOT NULL,
  `program_id` int NOT NULL,
  `program_name` varchar(32) NOT NULL,
  `cycle_count` int(11) NOT NULL,
  `cycle_total` int(11) NOT NULL,
  `start_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `finish_time` datetime,
  `op_id` int NOT NULL,
  `op_name` varchar(32) NOT NULL,
  `need_bio` tinyint NOT NULL DEFAULT 0, -- 0:no 1:yes
  `phy_check_result` tinyint(1) DEFAULT 0, -- 0:unchecked 1:unqualified 2:qualified
  `phy_check_time` datetime,
  `phy_check_op_id` int DEFAULT 0,
  `phy_check_op_name` varchar(32),
  `phy_check_photo` varchar(128),
  `che_check_result` tinyint(1) DEFAULT 0, -- 0:unchecked 1:unqualified 2:qualified
  `che_check_time` datetime,
  `che_check_op_id` int DEFAULT 0,
  `che_check_op_name` varchar(32),
  `che_check_photo` varchar(128),
  `bio_check_result` tinyint(1) DEFAULT 0, -- 0:unchecked 1:unqualified 2:qualified 3: uninvolved
  `bio_check_time` datetime,
  `bio_check_op_id` int DEFAULT 0,
  `bio_check_op_name` varchar(32),
  `bio_check_photo` varchar(128),
  `has_wet_pack` tinyint(1) DEFAULT 0, -- 0:no 1:yes
  `has_label_off` tinyint(1) DEFAULT 0, -- 0:no 1:yes
  KEY `idx_batch_id` (`batch_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `r_ster_package` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle` int unsigned NOT NULL,
  `pkg_name` varchar(32) NOT NULL,
  `batch_id` varchar(32) NOT NULL,
  `wet_pack` tinyint(1) DEFAULT 0, -- 0:no 1:yes
  KEY `idx_pkg_udi_cycle` (`pkg_udi`,`pkg_cycle`) USING BTREE,
  KEY `idx_batch_id` (`batch_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `r_dispatch` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle` int unsigned NOT NULL,
  `pkg_name` varchar(32) NOT NULL,
  `to_dept_id` int NOT NULL,
  `to_dept_name` varchar(32) NOT NULL,
  `op_id` int NOT NULL,
  `op_name` varchar(32) NOT NULL,
  `dispatch_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  KEY `idx_pkg_udi_cycle` (`pkg_udi`,`pkg_cycle`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `r_receive` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle` int unsigned NOT NULL,
  `pkg_name` varchar(32) NOT NULL,
  `rcv_dept_id` int NOT NULL,
  `rcv_dept_name` varchar(32) NOT NULL,
  `op_id` int NOT NULL,
  `op_name` varchar(32) NOT NULL,
  `rcv_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  KEY `idx_pkg_udi_cycle` (`pkg_udi`,`pkg_cycle`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `r_surgery` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `surgery_name` varchar(32) NOT NULL,
  `surgery_time` datetime NOT NULL,
  `room` varchar(32) NOT NULL,
  `patient_id` int NOT NULL,
  `patient_name` varchar(32) NOT NULL,
  `patient_age` tinyint,
  `patient_gender` tinyint(1),  -- 0:male 1:female
  -- `Diagnostic_id` varchar(32)
  `status` tinyint NOT NULL DEFAULT 0 -- 0:init 1:pre-checked 2:post-checked
) ENGINE=InnoDB AUTO_INCREMENT=810001 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `r_surgery_package` (
  `id` int unsigned PRIMARY KEY AUTO_INCREMENT,
  `surgery_id` int NOT NULL,
  `pkg_udi` varchar(32) NOT NULL,
  `pkg_cycle` int unsigned NOT NULL,
  `pkg_name` varchar(32) NOT NULL,
  KEY `idx_surgery_id` (`surgery_id`) USING BTREE,
  KEY `idx_pkg_udi_cycle` (`pkg_udi`,`pkg_cycle`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
