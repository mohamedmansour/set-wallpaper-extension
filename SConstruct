import os.path

# Create action object for exporting markdown files.
def markdown(target, source, env):
  import markdown
  markdown.markdownFromFile(input=source[0].abspath,
                            output=target[0].abspath)
markdown_action = Action(markdown, "Exporting Markdown from $SOURCE to $TARGET")


################################################################################
# Main script

# Define command-line arguments
vars = Variables()
vars.Add(BoolVariable('DEBUG', 'Build extension shared library in debug mode', False))
vars.Add(EnumVariable('TARGET_ARCH', 'Architecture to build for', 'x86', ['x86', 'x86_64']))

env = Environment(variables=vars)
env.Help(vars.GenerateHelpText(env, sort=cmp))

build_dir_name = 'build-' + ('debug' if env['DEBUG'] else 'release') + '-' + env['TARGET_ARCH']
install_dir_name = 'install-' + ('debug' if env['DEBUG'] else 'release') + '-' + env['TARGET_ARCH']

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

# Indicate default target is the creation of an unpacked extension.
env.Default(install_actions)

# Define a named target for exporting README.md from markdown to html.
readme = env.Command('README.html', 'README.md', markdown_action)
env.Alias('readme', readme)

# TODO: Add a target to use chrome to create a packaged extension.
