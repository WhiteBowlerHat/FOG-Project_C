# Projet Fog

Here is a simple flow chart:

```mermaid
flowchart LR;
    Clef-->CSPRNG;
    Nonce-->CSPRNG;
    B{0,0,0,0,...} e1@==> CSPRNG;
    CSPRNG --> C{1,9,4,2...};
    e1@{ animate: true }
```