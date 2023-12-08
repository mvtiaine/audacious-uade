CREATE OR REPLACE FUNCTION decode_url(p varchar) RETURNS varchar AS $$
SELECT convert_from(CAST(E'\\x' || string_agg(CASE WHEN length(r.m[1]) = 1 THEN encode(convert_to(r.m[1], 'SQL_ASCII'), 'hex') ELSE substring(r.m[1] from 2 for 2) END, '') AS bytea), 'UTF8')
FROM regexp_matches($1, '%[0-9a-f][0-9a-f]|.', 'gi') AS r(m);
$$ LANGUAGE SQL IMMUTABLE STRICT;

COPY (
SELECT DISTINCT a.production_id AS id, h.id AS prod_id, b.release_date_date AS mod_date, b.release_date_precision AS mod_date_precision, h.release_date_date AS prod_date, h.release_date_precision AS prod_date_precision, d.name AS mod_platform, array_agg(DISTINCT j.name) AS prod_platforms, h.title AS prod, decode_url(a.parameter) AS path, array_agg(DISTINCT s.name) AS authors, array_agg(DISTINCT f.name) AS mod_publishers, array_agg(DISTINCT l.name) AS prod_publishers, array_agg(DISTINCT m.original_url) AS image_urls, q.name AS party, o.shown_date_date AS party_date, o.shown_date_precision AS party_date_precision
    FROM productions_productionlink a, productions_production b
    LEFT JOIN productions_production_platforms c
        ON c.production_id = b.id
    LEFT JOIN platforms_platform d
        ON d.id = c.platform_id
    LEFT JOIN productions_production_author_affiliation_nicks e
        ON e.production_id = b.id
    LEFT JOIN demoscene_nick f
        ON f.id = e.nick_id
    LEFT JOIN productions_soundtracklink g
        ON g.soundtrack_id = b.id
    LEFT JOIN productions_production h
        ON h.id = g.production_id AND h.release_date_date <= b.release_date_date + 30
    LEFT JOIN productions_production_platforms i
        ON i.production_id = h.id
    LEFT JOIN platforms_platform j
        ON j.id = i.platform_id
    LEFT JOIN productions_production_author_nicks k
        ON k.production_id = h.id
    LEFT JOIN demoscene_nick l
        ON l.id = k.nick_id
    LEFT JOIN productions_screenshot m
        ON (m.production_id = b.id OR m.production_id = h.id)
    LEFT JOIN parties_competitionplacing n
        ON n.production_id = b.id
    LEFT JOIN parties_competition o
        ON o.id = n.competition_id
    LEFT JOIN parties_party p
        ON p.id = o.party_id
    LEFT JOIN parties_partyseries q
        ON q.id = p.party_series_id
    LEFT JOIN productions_production_author_nicks r
        ON r.production_id = b.id
    LEFT JOIN demoscene_nick s
        ON s.id = r.nick_id
    WHERE (a.link_class = 'ModlandFile' OR a.parameter like '%amp.dascene.net%') AND a.production_id = b.id
    GROUP BY a.production_id, h.id, b.release_date_date, b.release_date_precision, d.name, a.parameter, q.name, o.shown_date_date, o.shown_date_precision
) TO '/tmp/demozoo.tsv' WITH NULL AS '';
