# Projet Fog

Here is a simple flow chart:

```mermaid
graph TD;
    Clef syméttique-->CSPRNG;
    Nonce-->CSPRNG;
    CSRNG-->Sortie;
```