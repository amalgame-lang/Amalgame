/*
 * facade-stub.h — empty runtime header for the fake-facade fixture.
 *
 * The package's API is implemented in `facade.am` (pure Amalgame);
 * this file exists only so the manifest's `[stdlib].header` field —
 * required by PackageRegistry.LoadFrom — has something to point at.
 * The user binary's #include of this header is a no-op.
 */
