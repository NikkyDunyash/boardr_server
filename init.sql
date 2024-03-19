CREATE DATABASE boardb;
\c boardb;

CREATE TABLE users (
  username  VARCHAR(16) PRIMARY KEY,
  password CHAR(64),
  pfp BYTEA
);

CREATE TABLE comments (
  id SERIAL PRIMARY KEY, 
  username VARCHAR(16),
  date_time TIMESTAMP, 
  comment TEXT
);