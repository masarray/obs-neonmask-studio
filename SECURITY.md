# Security policy

## Supported versions

OBS NeonMask Studio is currently a **public preview**.

| Version | Security fixes |
| --- | --- |
| v0.1.x public previews | Best-effort while the preview line is active |
| Older development snapshots / CI artifacts | Not supported |

## Reporting a vulnerability

Please **do not open a public GitHub issue** for vulnerabilities that could put users at risk.

Use GitHub's private vulnerability-reporting / Security Advisory mechanism for this repository when available. If that option is not available to you, contact the repository maintainer privately through an account channel rather than publishing exploit details.

Please include:

- affected version/commit;
- OBS version and OS;
- impact and attack preconditions;
- minimal reproduction steps or proof of concept;
- whether untrusted files/settings are required;
- suggested mitigation, if known.

Avoid including stream keys, credentials, personal recordings, or unrelated private data.

## Security boundaries

NeonMask Studio is a native in-process OBS plugin. A plugin defect can affect the OBS process; the project does not claim process-level isolation.

The preview intentionally rejects or defers several high-risk extensibility surfaces:

- no arbitrary user shaders;
- no remote asset fetching;
- no browser-runtime dependency;
- restricted local SVG path subset with unsupported constructs rejected;
- no cloud account or marketplace integration.

Local files selected by the user should still be treated as untrusted input and validated within the documented parser/resource limits.

## Disclosure

Please allow reasonable time for triage and a fix before public disclosure. Confirmed fixes should identify the affected versions and remediation in the release notes or security advisory.
