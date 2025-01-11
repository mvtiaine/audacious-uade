-- SPDX-License-Identifier: GPL-2.0-or-later
-- Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

COPY (
    SELECT DISTINCT CONCAT('http://ftp.amigascne.org/pub/amiga',a.parameter) FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.link_class = 'AmigascneFile'
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_amigascne.tsv' WITH NULL AS '';

COPY (
    SELECT DISTINCT CONCAT('https://ftp.untergrund.net/users/ltk_tscc/fujiology',a.parameter) FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.link_class = 'FujiologyFile'
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_fujiology.tsv' WITH NULL AS '';

COPY (
    SELECT DISTINCT CONCAT('https://api.modarchive.org/downloads.php?moduleid=',a.parameter) FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.link_class = 'ModarchiveModule'
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_modarchive.tsv' WITH NULL AS '';

COPY (
    SELECT DISTINCT CONCAT('ftp://ftp.padua.org/pub/c64',a.parameter) FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.link_class = 'PaduaOrgFile'
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_padua.tsv' WITH NULL AS '';

COPY (
    SELECT DISTINCT CONCAT('https://files.scene.org/get',a.parameter) FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.link_class = 'SceneOrgFile'
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_sceneorg.tsv' WITH NULL AS '';

COPY (
    SELECT DISTINCT CONCAT('https://ftp.untergrund.net',a.parameter) FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.link_class = 'UntergrundFile'
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_untergrund.tsv' WITH NULL AS '';

COPY (
    SELECT DISTINCT CONCAT('https://web.archive.org/web/',a.parameter) FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.is_download_link = true
        AND a.link_class = 'WaybackMachinePage'
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_waybackmachine.tsv' WITH NULL AS '';

COPY (
    SELECT DISTINCT a.parameter FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.link_class = 'BaseUrl'
        AND a.parameter LIKE 'http%://media.demozoo.org/%'
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_media.tsv' WITH NULL AS '';

COPY (
    SELECT DISTINCT a.parameter FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    WHERE
        a.is_download_link = true
        AND a.link_class = 'BaseUrl'
        AND NOT (
            a.parameter LIKE 'http%://amp.dascene.net/%'
            OR a.parameter LIKE 'http%://aminet.net/%'
            OR a.parameter LIKE 'http%://wt.exotica.org.uk/files/%'
            OR a.parameter LIKE 'http%://files.exotica.org.uk/?file=exotica/media/audio/UnExoticA/%'
            OR a.parameter LIKE 'http%://www.exotica.org.uk/download.php?file=media/audio/UnExoticA/%'
            OR a.parameter LIKE 'http%://media.demozoo.org/%'
        )
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name like '%Music')
) TO '/tmp/demozoo_leftovers.tsv' WITH NULL AS '';
