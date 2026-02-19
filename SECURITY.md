# Security Policy

## Supported Versions

Use this section to tell people about which versions of your project are currently being supported with security updates.

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |
| < 1.0   | :x:                |

## Reporting a Vulnerability

We take the security of our software seriously. If you believe you have found a security vulnerability in the V2V Database engine, please report it to us as described below.

**Please do not report security vulnerabilities through public GitHub issues.**

Instead, please report them by email to [INSERT EMAIL ADDRESS]. You should receive a response within 48 hours. If for some reason you do not, please follow up via email to ensure we received your original message.

Please include the following details in your report:

*   Description of the vulnerability.
*   Steps to reproduce the issue.
*   Potential impact of the vulnerability.

We prefer all communications to be in English.

## Disclosure Policy

We follow a [responsible disclosure policy](https://en.wikipedia.org/wiki/Responsible_disclosure). We ask that you give us a reasonable amount of time to respond to your report and fix the issue before making it public.

## Security Features

V2V Database implements several security features by default:

1.  **Input Sanitization**: All identifiers (table names, column names) are automatically sanitized to remove special characters.
2.  **Path Traversal Protection**: Export and Import operations are restricted to the verified working directory.
3.  **Authentication**: Simple authentication ensures basic access control.
