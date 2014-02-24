--
-- compare.sql
--

ATTACH '../../../osm-testdata/grid/grid.db' AS grid;


SELECT 'Comparing multipolygons:';

.headers ON
.mode column
.width 10 10 10 10
SELECT g.from_type, g.id, g.variant, CASE Equals(m.geometry, g.geom) WHEN 1 THEN 'OK' ELSE 'DIFFER' END AS result FROM multipolygons m, grid.multipolygons g WHERE m.id=g.id AND m.from_type=g.from_type ORDER BY g.id;

SELECT 'Missing:';
SELECT from_type, id FROM multipolygons WHERE id NOT IN (SELECT id FROM grid.multipolygons);

