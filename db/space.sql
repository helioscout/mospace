DROP INDEX "sprite_key_unique";
DROP INDEX "world_label_unique";

DROP TABLE IF EXISTS "entity";
DROP TABLE IF EXISTS "position";
DROP TABLE IF EXISTS "rotation";
DROP TABLE IF EXISTS "sprite";
DROP TABLE IF EXISTS "tag";
DROP TABLE IF EXISTS "world";

CREATE TABLE "entity" (
  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "world_id" integer NOT NULL,
  "position_id" integer,
  "rotation_id" integer,
  "tag_id" integer,
  "sprite_key" TEXT NOT NULL,
  CONSTRAINT "fk_entity_world" FOREIGN KEY ("world_id") REFERENCES "world" ("id") ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT "fk_entity_position" FOREIGN KEY ("position_id") REFERENCES "position" ("id") ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT "fk_entity_rotation" FOREIGN KEY ("rotation_id") REFERENCES "rotation" ("id") ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT "fk_entity_tag" FOREIGN KEY ("tag_id") REFERENCES "tag" ("id") ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT "fk_entity_sprite" FOREIGN KEY ("sprite_key") REFERENCES "sprite" ("key") ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE "position" (
  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "x" integer NOT NULL,
  "y" integer NOT NULL
);

CREATE TABLE "rotation" (
  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "angle" real NOT NULL DEFAULT 0.0
);

CREATE TABLE "sprite" (
  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "key" TEXT NOT NULL,
  "label" TEXT NOT NULL,
  "file_name" TEXT NOT NULL
);
CREATE UNIQUE INDEX "sprite_key_unique"
ON "sprite" (
  "key"
);

CREATE TABLE "tag" (
  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "player" integer NOT NULL DEFAULT 0,
  "asteroid" integer NOT NULL DEFAULT 0,
  "enemy" integer NOT NULL DEFAULT 0
);

CREATE TABLE "world" (
  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "label" TEXT NOT NULL,
  "width" integer NOT NULL,
  "height" integer NOT NULL
);
CREATE UNIQUE INDEX "world_label_unique"
ON "world" (
  "label"
);

