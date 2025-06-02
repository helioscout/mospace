DROP INDEX "sprites_key_unique";

DROP TABLE IF EXISTS "sprite";

CREATE TABLE "sprite" (
  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "key" TEXT NOT NULL,
  "label" TEXT NOT NULL,
  "file_name" TEXT NOT NULL
);
CREATE UNIQUE INDEX "sprites_key_unique"
ON "sprite" (
  "key"
);

