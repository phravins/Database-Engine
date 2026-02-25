-- Security Test Suite
-- 1. Test Auto Removal (Sanitization)
-- Expected: Table created as 'securitytest'
CREATE TABLE "security;test" id INT

-- 2. Test Path Traversal Protection
-- Expected: Fail with Security Error
EXPORT securitytest ../hacked.csv

-- 3. Test Valid Export
-- Expected: Success
INSERT INTO securitytest VALUES 1
EXPORT securitytest valid_security_test.csv
