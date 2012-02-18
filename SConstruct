import os.path

# Create action object for exporting markdown files.
def markdown(target, source, env):
  import markdown
  markdown.markdownFromFile(input=source[0].abspath,
                            output=target[0].abspath)
markdown_action = Action(markdown, "Exporting Markdown from $SOURCE to $TARGET")

def package_extension_func(target, source, env):
  import subprocess

  if 'CHROME_BIN' not in env or not os.path.isfile(env['CHROME_BIN']):
    import SCons.Errors
    raise SCons.Errors.BuildError(errstr='CHROME_BIN not provided or doesn\'t exist.')

  args = [env['CHROME_BIN'],
          '--no-message-box',
          '--pack-extension={0}'.format(source[0].abspath)]

  # If the signing key isn't provided, don't specify one. Chrome will create a new one.
  if len(target) == 1:
    args.append('--pack-extension-key={0}'.format(os.path.abspath(env['PRIVATE_KEY'])))

  subprocess.check_call(args)

# Targets are determined implicitly from the source (which must be a
# directory). If no signing key is provided, add it to the target list as
# chrome will create one. Normally, chrome will refuse to perform the packaging
# if it means a newly created signing key would overwrite an existing one.
# However, by adding the signing key to the target list, SCons seems to delete
# all targets if it deems a build is necessary thus there will never be a key
# to overwrite.
def extension_emitter(target, source, env):
  (root_dir, extension_name) = os.path.split(source[0].path)

  targets = [os.path.join(root_dir, extension_name + '.crx')]

  if 'PRIVATE_KEY' not in env:
    targets.append(os.path.join(root_dir, extension_name + '.pem'))
  return (targets, source)

# Builder for packaging extensions. Requires the construction variable 'CHROME_BIN'
# to be set. Optional: 'PRIVATE_KEY'.
extension_builder = Builder(action=Action(package_extension_func,
                                          "Creating extension $TARGET from root directory $SOURCE"),
                            source_factory=Dir,
                            emitter=extension_emitter)

# Our zip builder is simpler than SCon's built-in one in that we assume there
# is a single source and it's a directory. However, this builder does something
# the built-in builder doesn't: store files in the zip with different name/path
# than what's found on disk. Files within the zip are rooted in a directory
# with the same name as the last component of the path of source[0].
def zip_func(target, source, env):
  import zipfile

  zip_root = os.path.split(str(source[0]))[0]

  zf = zipfile.ZipFile(str(target[0]),'w', zipfile.ZIP_DEFLATED)
  for root, dirs, files in os.walk(str(source[0])):
    for f in files:
      full_path = os.path.join(root, f)
      zf.write(os.path.join(root, f), os.path.relpath(full_path, zip_root))

zipper = Builder(action = Action(zip_func, "Creating zipfile $TARGET from directory $SOURCE"),
                 source_factory = Dir,
                 suffix = '.zip')

################################################################################
# Main script

extension_name = 'set-wallpaper-extension'

# Define command-line arguments
vars = Variables()
vars.Add(BoolVariable('DEBUG', 'Build extension shared library in debug mode', False))
vars.Add(EnumVariable('TARGET_ARCH', 'Architecture to build for', 'x86', ['x86', 'x86_64']))

# We don't care about these arguments unless we're choosing to package the
# extension. We could use validators that check to see what target is being
# used but this is fragile (target names may change). So instead, the presence
# of these arguments (CHROME_BIN is the only mandatory one) is tested by the
# extension builder.
vars.Add(PathVariable('CHROME_BIN', 'Location of the chrome executable', None, PathVariable.PathAccept))
vars.Add(PathVariable('PRIVATE_KEY', 'Location of the private signing key (.pem file) for this extension', None, PathVariable.PathAccept))

env = Environment(variables=vars)
env.Help(vars.GenerateHelpText(env, sort=cmp))
env.Append(BUILDERS = {'ExtensionPackager':extension_builder})
env.Append(BUILDERS = {'Zipper' : zipper})

build_dir_name = 'build-' + ('debug' if env['DEBUG'] else 'release') + '-' + env['TARGET_ARCH']
install_dir_name = 'install-' + ('debug' if env['DEBUG'] else 'release') + '-' + env['TARGET_ARCH']
install_dir_name = os.path.join(install_dir_name, extension_name)

# Read source/SConscript which defines how to build the shared library.
Export('env')
shared_lib = env.SConscript(os.path.join('source', 'SConscript'), variant_dir = build_dir_name, duplicate=0)

# Define steps necessary to put together an 'unpacked' extension.
install_actions = [env.Install(install_dir_name, shared_lib[0]),
                   env.Install(install_dir_name, env.File('manifest.json')),
                   env.Install(install_dir_name, env.Dir('css')),
                   env.Install(install_dir_name, env.Dir('js')),
                   env.Install(install_dir_name, env.Dir('img')),
                   env.Install(install_dir_name, env.Glob('*.html'))]

env.Alias('unpacked', install_actions)
env.Default(install_actions)

# Target to create a packed and signed extension.
pack = env.ExtensionPackager(install_dir_name)
env.Alias('packed', pack)

# Target to create a zip file of the unpacked extension.
zipped = env.Zipper('set-wallpaper-extension.zip', install_dir_name)
env.Alias('zip', zipped)

# Target processing README.md into html.
readme = env.Command('README.html', 'README.md', markdown_action)
env.Alias('readme', readme)
