CREATE OR REPLACE FUNCTION decode_url(p varchar) RETURNS varchar AS $$
BEGIN
    RETURN(SELECT convert_from(CAST(E'\\x' || string_agg(CASE WHEN length(r.m[1]) = 1 THEN encode(convert_to(r.m[1], 'SQL_ASCII'), 'hex') ELSE substring(r.m[1] from 2 for 2) END, '') AS bytea), 'UTF8')
    FROM regexp_matches($1, '%[0-9a-f][0-9a-f]|.', 'gi') AS r(m));
EXCEPTION WHEN OTHERS THEN
    RAISE NOTICE 'invalid url %', $1;
    RETURN $1;
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;

COPY (
SELECT DISTINCT
        a.production_id AS id,
        i.id AS prod_id,
        b.release_date_date AS mod_date,
        b.release_date_precision AS mod_date_precision,
        i.release_date_date AS prod_date,
        i.release_date_precision AS prod_date_precision,
        e.name AS mod_platform,
        array_agg(DISTINCT k.name) AS prod_platforms,
        i.title AS prod,
        a.link_class AS link_class,
        decode_url(a.parameter) AS path,
        array_agg(DISTINCT t.name) AS authors,
        array_agg(DISTINCT g.name) AS mod_publishers,
        array_agg(DISTINCT m.name) AS prod_publishers,
--        array_agg(DISTINCT n.original_url) AS image_urls,
        r.name AS party,
        p.shown_date_date AS party_date,
        p.shown_date_precision AS party_date_precision
    FROM
        productions_production_types c,
        productions_productionlink a,
        productions_production b
    LEFT JOIN productions_production_platforms d
        ON d.production_id = b.id
    LEFT JOIN platforms_platform e
        ON e.id = d.platform_id
    LEFT JOIN productions_production_author_affiliation_nicks f
        ON f.production_id = b.id
    LEFT JOIN demoscene_nick g
        ON g.id = f.nick_id
    LEFT JOIN productions_soundtracklink h
        ON h.soundtrack_id = b.id
    LEFT JOIN productions_production i
        ON i.id = h.production_id AND i.release_date_date <= b.release_date_date + 30
    LEFT JOIN productions_production_platforms j
        ON j.production_id = i.id
    LEFT JOIN platforms_platform k
        ON k.id = j.platform_id
    LEFT JOIN productions_production_author_nicks l
        ON l.production_id = i.id
    LEFT JOIN demoscene_nick m
        ON m.id = l.nick_id
    LEFT JOIN productions_screenshot n
        ON (n.production_id = b.id OR n.production_id = i.id)
    LEFT JOIN parties_competitionplacing o
        ON o.production_id = b.id
    LEFT JOIN parties_competition p
        ON p.id = o.competition_id
    LEFT JOIN parties_party q
        ON q.id = p.party_id
    LEFT JOIN parties_partyseries r
        ON r.id = q.party_series_id
    LEFT JOIN productions_production_author_nicks s
        ON s.production_id = b.id
    LEFT JOIN demoscene_nick t
        ON t.id = s.nick_id
    WHERE
        (
            (a.is_download_link = true AND a.link_class = 'BaseUrl')
            OR a.link_class = 'AmigascneFile'
            OR a.link_class = 'FujiologyFile'
            OR a.link_class = 'ModarchiveModule'
            OR a.link_class = 'ModlandFile'
            OR a.link_class = 'PaduaOrgFile'
            OR a.link_class = 'SceneOrgFile'
            OR a.link_class = 'UntergrundFile'
            OR a.link_class = 'WaybackMachinePage'
            OR a.parameter LIKE 'http%://amp.dascene.net/%'
            OR a.parameter LIKE 'http%://aminet.net/%'
            OR a.parameter LIKE 'http%://wt.exotica.org.uk/files/%'
            OR a.parameter LIKE 'http%://files.exotica.org.uk/?file=exotica/media/audio/UnExoticA/%'
            OR a.parameter LIKE 'http%://www.exotica.org.uk/download.php?file=media/audio/UnExoticA/%'
            OR a.parameter LIKE 'http%://media.demozoo.org/%'
        )
        AND a.production_id = b.id
        AND c.production_id = b.id
        AND c.productiontype_id IN (SELECT id FROM productions_productiontype WHERE name LIKE '%Music')
    GROUP BY
        a.production_id,
        i.id,
        b.release_date_date,
        b.release_date_precision,
        e.name,
        a.link_class,
        a.parameter,
        r.name,
        p.shown_date_date,
        p.shown_date_precision
) TO '/tmp/demozoo.tsv' WITH NULL AS '';
