# Setup Coding Agent Environment

This GitHub Action provides a consistent environment setup for quality checks across all workflows, including GitHub Copilot Coding Agent environments.

## Purpose

The Coding Agent runs in isolated GitHub-managed environments that don't have access to:

- Dev Container initialization scripts (`.devcontainer/post-start.sh`)
- Pre-installed tools from Dockerfile
- Local virtualenv setups

This action ensures **all quality tools are available** in CI/CD pipelines, making quality gates enforceable for both human developers and AI agents.

## Installed Tools

### Python Tools

- **pre-commit**: Git hook framework for quality checks
- **mkdocs**: Documentation site generator
- **mkdocs-material**: Material Design theme for MkDocs
- **mkdocstrings**: API documentation generator
- **pymdown-extensions**: Markdown extensions

### Node.js Tools

- **markdownlint-cli**: Markdown linting and style checking

## Usage

### In Workflows

```yaml
steps:
  - uses: actions/checkout@v4
  
  - name: Setup environment
    uses: ./.github/actions/setup-coding-agent-env
  
  - name: Run quality checks
    run: pre-commit run --all-files
```

### In Pre-commit CI

This action is automatically used by `.github/workflows/pre-commit.yml`:

```yaml
jobs:
  quality-checks:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: ./.github/actions/setup-coding-agent-env
      - run: pre-commit run --all-files
```

## Caching

The action leverages GitHub Actions caching:

- **Python**: `pip` cache via `actions/setup-python@v5`
- **Node.js**: `npm` cache via `actions/setup-node@v4`
- **Pre-commit hooks**: Cached separately in workflows using `actions/cache@v4`

## Benefits

### For Coding Agents

✅ **Consistent environment** across all PR builds
✅ **Automatic tool installation** without manual setup
✅ **Quality gates enforced** regardless of local environment
✅ **Fast feedback** through CI checks

### For Developers

✅ **Same tools locally and in CI** via dev container
✅ **No environment drift** between dev and CI
✅ **Easy debugging** of CI failures locally

### For Project

✅ **Enforces quality standards** automatically
✅ **Prevents broken documentation** from merging
✅ **Reduces review burden** by catching trivial errors
✅ **Maintains consistency** across contributors

## Maintenance

### Updating Tool Versions

Edit `action.yml` to update version constraints:

```yaml
- name: Install Python tools
  run: |
    pip install --no-cache-dir \
      pre-commit>=3.5 \
      mkdocs>=1.6 \
      mkdocs-material>=9.5
```

### Testing Changes

Test the action locally using [act](https://github.com/nektos/act):

```bash
act pull_request -j quality-checks
```

Or push to a test branch and observe CI:

```bash
git checkout -b test/action-update
git push origin test/action-update
```

## Integration with Project

### Related Files

- `.github/workflows/pre-commit.yml`: Main quality gate workflow
- `.pre-commit-config.yaml`: Pre-commit hook configuration
- `.devcontainer/post-start.sh`: Local dev environment setup
- `docs/development/pre-commit-hooks.md`: Documentation

### Local Development

Developers using the dev container automatically get these tools via:

1. **Dockerfile**: Installs tools system-wide
2. **post-start.sh**: Sets up pre-commit hooks

This action ensures **CI matches local environment**.

## Troubleshooting

### Action Fails to Install Tools

**Error**: `pip install failed` or `npm install failed`

**Solution**: Check if PyPI or npm registry is accessible

### Pre-commit Hooks Fail

**Error**: `markdownlint not found`

**Solution**: Verify Node.js setup step completed successfully

### Slow CI Builds

**Solution**: Ensure caching is working:

```yaml
- uses: actions/cache@v4
  with:
    path: ~/.cache/pre-commit
    key: pre-commit-${{ hashFiles('.pre-commit-config.yaml') }}
```

## References

- [GitHub Documentation: Customize the agent environment](https://docs.github.com/en/copilot/how-tos/use-copilot-agents/coding-agent/customize-the-agent-environment)
- [Pre-commit Framework](https://pre-commit.com/)
- [GitHub Actions: Creating actions](https://docs.github.com/en/actions/creating-actions)
