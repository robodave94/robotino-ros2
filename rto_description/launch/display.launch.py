from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.launch_description_sources import PythonLaunchDescriptionSource
import pdb


def generate_launch_description():
    ld = LaunchDescription()

    urdf_tutorial_path = FindPackageShare('rto_description')
    default_model_path = PathJoinSubstitution([urdf_tutorial_path, 'urdf', 'robots', 'rto-3.urdf.xacro'])
    default_rviz_config_path = PathJoinSubstitution([urdf_tutorial_path, 'config', 'urdf.rviz'])

    # These parameters are maintained for backwards compatibility
    gui_arg = DeclareLaunchArgument(name='gui', default_value='true', choices=['true', 'false'],
                                    description='Flag to enable joint_state_publisher_gui')
    ld.add_action(gui_arg)
    rviz_arg = DeclareLaunchArgument(name='rvizconfig', default_value=default_rviz_config_path,
                                     description='Absolute path to rviz config file')
    ld.add_action(rviz_arg)

    # This parameter has changed its meaning slightly from previous versions
    ld.add_action(DeclareLaunchArgument(name='model', default_value=default_model_path,
                                        description='Path to robot urdf file relative to urdf_tutorial package'))

    l_args = {
            'urdf_package': 'rto_description',
            'urdf_package_path': LaunchConfiguration('model'),
            'rviz_config': LaunchConfiguration('rvizconfig'),
            'jsp_gui': LaunchConfiguration('gui')}

    ld.add_action(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution([FindPackageShare('urdf_launch'), 'launch', 'display.launch.py'])),
        launch_arguments=l_args.items()
    ))

    return ld